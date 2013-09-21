#ifdef FEATURE_OEM_DATA_SUPPORT

/**===========================================================================
  
  \file  wlan_hdd_oemdata.h
  
  \brief Internal includes for the oem data
  
               Copyright 2008 (c) Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary.
  
  ==========================================================================*/


#ifndef __WLAN_HDD_OEM_DATA_H__
#define __WLAN_HDD_OEM_DATA_H__

#ifndef OEM_DATA_REQ_SIZE
#ifdef QCA_WIFI_2_0
#define OEM_DATA_REQ_SIZE 276
#else
#define OEM_DATA_REQ_SIZE 134
#endif
#endif

#ifndef OEM_DATA_RSP_SIZE
#ifdef QCA_WIFI_2_0
#define OEM_DATA_RSP_SIZE 1720
#else
#define OEM_DATA_RSP_SIZE 1968
#endif
#endif

#ifdef QCA_WIFI_2_0
#define OEM_APP_SIGNATURE_LEN      16
#define OEM_APP_SIGNATURE_STR      "QUALCOMM-OEM-APP"

#define OEM_TARGET_SIGNATURE_LEN   8
#define OEM_TARGET_SIGNATURE       "QUALCOMM"

typedef enum
{
  /* Error null context */
  OEM_ERR_NULL_CONTEXT = 1,

  /* OEM App is not registered */
  OEM_ERR_APP_NOT_REGISTERED,

  /* Inavalid signature */
  OEM_ERR_INVALID_SIGNATURE,

  /* Invalid message type */
  OEM_ERR_NULL_MESSAGE_HEADER,

  /* Invalid message type */
  OEM_ERR_INVALID_MESSAGE_TYPE,

  /* Invalid length in message body */
  OEM_ERR_INVALID_MESSAGE_LENGTH
} eOemErrorCode;

int oem_activate_service(void *pAdapter);

int iw_get_oem_data_cap(struct net_device *dev, struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra);

typedef struct sDriverVersion
{
  tANI_U8 major;
  tANI_U8 minor;
  tANI_U8 patch;
  tANI_U8 build;
} tDriverVersion;

struct iw_oem_data_cap
{
    /* Signature of chipset vendor, e.g. QUALCOMM */
    tANI_U8 oem_target_signature[OEM_TARGET_SIGNATURE_LEN];
    tANI_U32 oem_target_type;         /* Chip type */
    tANI_U32 oem_fw_version;          /* FW version */
    tDriverVersion driver_version;    /* CLD version */
    tANI_U16 allowed_dwell_time_min;  /* Channel dwell time - allowed min */
    tANI_U16 allowed_dwell_time_max;  /* Channel dwell time - allowed max */
    tANI_U16 curr_dwell_time_min;     /* Channel dwell time - current min */
    tANI_U16 curr_dwell_time_max;     /* Channel dwell time - current max */
    tANI_U8 supported_bands;          /* 2.4G or 5G Hz */
    tANI_U8 num_channels;             /* Num of channels IDs to follow */
    tANI_U8 *channel_list;            /* List of channel IDs */
};
#endif /* QCA_WIFI_2_0 */

struct iw_oem_data_req
{
    v_U8_t                  oemDataReq[OEM_DATA_REQ_SIZE];
};

int iw_set_oem_data_req(
        struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra);

int iw_get_oem_data_rsp(
        struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra);

struct iw_oem_data_rsp
{
    tANI_U8           oemDataRsp[OEM_DATA_RSP_SIZE];
};

#endif //__WLAN_HDD_OEM_DATA_H__

#endif //FEATURE_OEM_DATA_SUPPORT
