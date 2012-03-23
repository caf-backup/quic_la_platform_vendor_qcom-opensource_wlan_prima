#ifdef FEATURE_INNAV_SUPPORT

#ifndef __WLAN_HDD_INNAV_H__
#define __WLAN_HDD_INNAV_H__

#define MAX_BSSIDS_ALLOWED_FOR_MEASUREMENTS 8
#define MAX_MEASUREMENTS_ALLOWED_PER_BSSID 8
#define BSSID_SIZE 6

struct BSSIDChannelInfo
{
    tSirMacAddr     bssid; //BSSID of the AP to be measured
    v_U16_t         channelNumber; //Channel number of the AP to be measured
};

typedef enum
{
    RTS_CTS_MODE = 1,
    FRAME_BASED,
} eMeasurementMode;

struct iw_innav_measurement_request
{
    v_U8_t                  numBSSIDs; //number of BSSIDs in the measurement set
    v_U8_t                  numInNavMeasurements; //number of rtt/rssi measurements per BSSID
    v_U16_t                 numSetRepetitions; //Number of times to measure the given BSSID set
    v_U32_t                 measurementTimeInterval; //Time interval between the measurement sets
    eMeasurementMode        measurementMode; //Indicates whether to use RTS/CTS or frame based measurements
    struct BSSIDChannelInfo bssidChannelInfo[MAX_BSSIDS_ALLOWED_FOR_MEASUREMENTS]; //array of BSSID and channel info //fix it later
};

int iw_set_innav_measurements(
        struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra);

int iw_get_innav_measurements(
        struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra);

//Bit optimized structure for reducing the
//IOCTL data transnfer size
struct iw_innav_rtt_rssi_snr_data
{
#ifdef ANI_BIG_BYTE_ENDIAN
    unsigned int      rssi                 :  7;
    unsigned int      rtt                  : 10;
    unsigned int      snr                  :  8;
    unsigned int      reserved             :  7;
#else
    unsigned int      reserved             :  7;
    unsigned int      snr                  :  8;
    unsigned int      rtt                  : 10;
    unsigned int      rssi                 :  7;
#endif
    unsigned int      measurementTimeLo        ;
    unsigned int      measurementTimeHi        ;
};

struct iw_innav_results
{
    tSirMacAddr                        bssid;
    tANI_U8                            numSuccessfulMeasurements;
    struct iw_innav_rtt_rssi_snr_data  rttRssiSnrTimeData[1];
};

struct iw_innav_measurement_response
{
    tANI_U8                     numBSSIDs; //Number of BSSIDs for which the measurement was performed
    struct iw_innav_results     perBssidResultData[1]; //Pointer to the array of result data for each BSSID
};

#endif //__WLAN_HDD_INNAV_H__

#endif //FEATURE_INNAV_SUPPORT
