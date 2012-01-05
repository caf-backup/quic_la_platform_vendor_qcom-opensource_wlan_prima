#if !defined( HDD_CONFIG_H__ )
#define HDD_CONFIG_H__

/**===========================================================================
  
  \file  hdd_Config.h
  
  \brief Android WLAN Adapter Configuration functions
    
               Copyright 2008 (c) Qualcomm, Incorporated.
               All Rights Reserved.
               Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/* $HEADER$ */
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <wlan_hdd_includes.h>
#include <wlan_hdd_wmm.h>
#include <vos_types.h>
#include <csrApi.h>

//Number of items that can be configured
#define MAX_CFG_INI_ITEMS   150

// Defines for all of the things we read from the configuration (registry).

#define CFG_RTS_THRESHOLD_NAME                 "RTSThreshold"
#define CFG_RTS_THRESHOLD_MIN                  WNI_CFG_RTS_THRESHOLD_STAMIN // min is 0, meaning always use RTS.
#define CFG_RTS_THRESHOLD_MAX                  WNI_CFG_RTS_THRESHOLD_STAMAX // max is the max frame size
#define CFG_RTS_THRESHOLD_DEFAULT              WNI_CFG_RTS_THRESHOLD_STADEF

#define CFG_FRAG_THRESHOLD_NAME                "gFragmentationThreshold"
#define CFG_FRAG_THRESHOLD_MIN                 WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN 
#define CFG_FRAG_THRESHOLD_MAX                 WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX
#define CFG_FRAG_THRESHOLD_DEFAULT             WNI_CFG_FRAGMENTATION_THRESHOLD_STADEF

#define CFG_CALIBRATION_NAME                   "gCalibration"
#define CFG_CALIBRATION_MIN                    ( 0 ) 
#define CFG_CALIBRATION_MAX                    ( 1 )
#define CFG_CALIBRATION_MAC_DEFAULT            ( 1 )
#define CFG_CALIBRATION_DEFAULT                CFG_CALIBRATION_MAC_DEFAULT

#define CFG_CALIBRATION_PERIOD_NAME            "gCalibrationPeriod"
#define CFG_CALIBRATION_PERIOD_MIN             ( 2 ) 
#define CFG_CALIBRATION_PERIOD_MAX             ( 10 )
#define CFG_CALIBRATION_PERIOD_MAC_DEFAULT     ( 5 )
#define CFG_CALIBRATION_PERIOD_DEFAULT         CFG_CALIBRATION_PERIOD_MAC_DEFAULT          

#define CFG_OPERATING_CHANNEL_NAME             "gOperatingChannel"
#define CFG_OPERATING_CHANNEL_MIN              ( 0 )
#define CFG_OPERATING_CHANNEL_MAX              ( 14 )
#define CFG_OPERATING_CHANNEL_DEFAULT          ( 1 )

#define CFG_SHORT_SLOT_TIME_ENABLED_NAME       "gShortSlotTimeEnabled"
#define CFG_SHORT_SLOT_TIME_ENABLED_MIN        WNI_CFG_SHORT_SLOT_TIME_STAMIN
#define CFG_SHORT_SLOT_TIME_ENABLED_MAX        WNI_CFG_SHORT_SLOT_TIME_STAMAX
#define CFG_SHORT_SLOT_TIME_ENABLED_DEFAULT    WNI_CFG_SHORT_SLOT_TIME_STADEF

#define CFG_11D_SUPPORT_ENABLED_NAME           "g11dSupportEnabled"
#define CFG_11D_SUPPORT_ENABLED_MIN            WNI_CFG_11D_ENABLED_STAMIN 
#define CFG_11D_SUPPORT_ENABLED_MAX            WNI_CFG_11D_ENABLED_STAMAX 
#define CFG_11D_SUPPORT_ENABLED_DEFAULT        WNI_CFG_11D_ENABLED_STADEF    // Default is ON 

#define CFG_ENFORCE_11D_CHANNELS_NAME          "gEnforce11dChannel"
#define CFG_ENFORCE_11D_CHANNELS_MIN           ( 0 )
#define CFG_ENFORCE_11D_CHANNELS_MAX           ( 1 )
#define CFG_ENFORCE_11D_CHANNELS_DEFAULT       ( 0 )

#define CFG_ENFORCE_COUNTRY_CODE_MATCH_NAME    "gEnforceCountryCodeMatch"
#define CFG_ENFORCE_COUNTRY_CODE_MATCH_MIN     ( 0 )
#define CFG_ENFORCE_COUNTRY_CODE_MATCH_MAX     ( 1 )
#define CFG_ENFORCE_COUNTRY_CODE_MATCH_DEFAULT ( 0 )

#define CFG_ENFORCE_DEFAULT_DOMAIN_NAME        "gEnforceDefaultDomain"
#define CFG_ENFORCE_DEFAULT_DOMAIN_MIN         ( 0 )
#define CFG_ENFORCE_DEFAULT_DOMAIN_MAX         ( 1 ) 
#define CFG_ENFORCE_DEFAULT_DOMAIN_DEFAULT     ( 0 )

#define CFG_GENERIC_ID1_NAME                   "gCfg1Id"
#define CFG_GENERIC_ID1_MIN                    ( 0 )
#define CFG_GENERIC_ID1_MAX                    ( 0xffffffff )
#define CFG_GENERIC_ID1_DEFAULT                ( 0 )

#define CFG_GENERIC_ID2_NAME                   "gCfg2Id"
#define CFG_GENERIC_ID2_MIN                    ( 0 )
#define CFG_GENERIC_ID2_MAX                    ( 0xffffffff )
#define CFG_GENERIC_ID2_DEFAULT                ( 0 )

#define CFG_GENERIC_ID3_NAME                   "gCfg3Id"
#define CFG_GENERIC_ID3_MIN                    ( 0 )
#define CFG_GENERIC_ID3_MAX                    ( 0xffffffff )
#define CFG_GENERIC_ID3_DEFAULT                ( 0 )

#define CFG_GENERIC_ID4_NAME                   "gCfg4Id"
#define CFG_GENERIC_ID4_MIN                    ( 0 )
#define CFG_GENERIC_ID4_MAX                    ( 0xffffffff )
#define CFG_GENERIC_ID4_DEFAULT                ( 0 )

#define CFG_GENERIC_ID5_NAME                   "gCfg5Id"
#define CFG_GENERIC_ID5_MIN                    ( 0 )
#define CFG_GENERIC_ID5_MAX                    ( 0xffffffff )
#define CFG_GENERIC_ID5_DEFAULT                ( 0 )

#define CFG_GENERIC_VALUE1_NAME                "gCfg1Value"
#define CFG_GENERIC_VALUE1_MIN                 ( 0 )
#define CFG_GENERIC_VALUE1_MAX                 ( 0xffffffff )
#define CFG_GENERIC_VALUE1_DEFAULT             ( 0 )

#define CFG_GENERIC_VALUE2_NAME                "gCfg2Value"
#define CFG_GENERIC_VALUE2_MIN                 ( 0 )
#define CFG_GENERIC_VALUE2_MAX                 ( 0xffffffff )
#define CFG_GENERIC_VALUE2_DEFAULT             ( 0 )

#define CFG_GENERIC_VALUE3_NAME                "gCfg3Value"
#define CFG_GENERIC_VALUE3_MIN                 ( 0 )
#define CFG_GENERIC_VALUE3_MAX                 ( 0xffffffff )
#define CFG_GENERIC_VALUE3_DEFAULT             ( 0 )

#define CFG_GENERIC_VALUE4_NAME                "gCfg4Value"
#define CFG_GENERIC_VALUE4_MIN                 ( 0 )
#define CFG_GENERIC_VALUE4_MAX                 ( 0xffffffff )
#define CFG_GENERIC_VALUE4_DEFAULT             ( 0 )

#define CFG_GENERIC_VALUE5_NAME                "gCfg5Value"
#define CFG_GENERIC_VALUE5_MIN                 ( 0 )
#define CFG_GENERIC_VALUE5_MAX                 ( 0xffffffff )
#define CFG_GENERIC_VALUE5_DEFAULT             ( 0 )

#define CFG_HEARTBEAT_THRESH_24_NAME           "gHeartbeat24"
#define CFG_HEARTBEAT_THRESH_24_MIN            WNI_CFG_HEART_BEAT_THRESHOLD_STAMIN
#define CFG_HEARTBEAT_THRESH_24_MAX            WNI_CFG_HEART_BEAT_THRESHOLD_STAMAX
#define CFG_HEARTBEAT_THRESH_24_DEFAULT        WNI_CFG_HEART_BEAT_THRESHOLD_STADEF

#define CFG_POWER_USAGE_NAME                   "gPowerUsage"
#define CFG_POWER_USAGE_MIN                    "Min" //Minimum Power Save
#define CFG_POWER_USAGE_MAX                    "Max" //Maximum Power Save
#define CFG_POWER_USAGE_DEFAULT                "Mod" //Moderate Power Save

//Enable suspend on Android
#define CFG_ENABLE_SUSPEND_NAME                "gEnableSuspend"
#define CFG_ENABLE_SUSPEND_MIN                 ( 0 ) //No support for suspend
#define CFG_ENABLE_SUSPEND_MAX                 ( 3 ) //Map to Deep Sleep
#define CFG_ENABLE_SUSPEND_DEFAULT             ( 1 ) //Map to Standby

//Driver start/stop command mappings
#define CFG_ENABLE_ENABLE_DRIVER_STOP_NAME     "gEnableDriverStop"
#define CFG_ENABLE_ENABLE_DRIVER_STOP_MIN      ( 0 ) //No support for stop
#define CFG_ENABLE_ENABLE_DRIVER_STOP_MAX      ( 2 ) //Map to Deep Sleep
#define CFG_ENABLE_ENABLE_DRIVER_STOP_DEFAULT  ( 1 ) //Map to Standby

#define CFG_WOWL_PATTERN_NAME                  "gWowlPattern"
#define CFG_WOWL_PATTERN_DEFAULT               ""

//IMPS = IdleModePowerSave
#define CFG_ENABLE_IMPS_NAME                   "gEnableImps"
#define CFG_ENABLE_IMPS_MIN                    ( 0 )
#define CFG_ENABLE_IMPS_MAX                    ( 1 )
#define CFG_ENABLE_IMPS_DEFAULT                ( 1 )

#define CFG_IMPS_MINIMUM_SLEEP_TIME_NAME       "gImpsMinSleepTime" 
#define CFG_IMPS_MINIMUM_SLEEP_TIME_MIN        ( 0 )
#define CFG_IMPS_MINIMUM_SLEEP_TIME_MAX        ( 65535 )
#define CFG_IMPS_MINIMUM_SLEEP_TIME_DEFAULT    ( 5 )
    
#define CFG_IMPS_MODERATE_SLEEP_TIME_NAME      "gImpsModSleepTime"
#define CFG_IMPS_MODERATE_SLEEP_TIME_MIN       ( 0 )
#define CFG_IMPS_MODERATE_SLEEP_TIME_MAX       ( 65535 )
#define CFG_IMPS_MODERATE_SLEEP_TIME_DEFAULT   ( 10)

#define CFG_IMPS_MAXIMUM_SLEEP_TIME_NAME       "gImpsMaxSleepTime"
#define CFG_IMPS_MAXIMUM_SLEEP_TIME_MIN        ( 0 )
#define CFG_IMPS_MAXIMUM_SLEEP_TIME_MAX        ( 65535 )
#define CFG_IMPS_MAXIMUM_SLEEP_TIME_DEFAULT    ( 15 )

//BMPS = BeaconModePowerSave
#define CFG_ENABLE_BMPS_NAME                   "gEnableBmps"
#define CFG_ENABLE_BMPS_MIN                    ( 0 )
#define CFG_ENABLE_BMPS_MAX                    ( 1 )
#define CFG_ENABLE_BMPS_DEFAULT                ( 1 )

#define CFG_BMPS_MINIMUM_LI_NAME               "gBmpsMinListenInterval"
#define CFG_BMPS_MINIMUM_LI_MIN                ( 1 )
#define CFG_BMPS_MINIMUM_LI_MAX                ( 65535 )
#define CFG_BMPS_MINIMUM_LI_DEFAULT            ( 1 )
    
#define CFG_BMPS_MODERATE_LI_NAME              "gBmpsModListenInterval"
#define CFG_BMPS_MODERATE_LI_MIN               ( 1 )
#define CFG_BMPS_MODERATE_LI_MAX               ( 65535 )
#define CFG_BMPS_MODERATE_LI_DEFAULT           ( 1 )

#define CFG_BMPS_MAXIMUM_LI_NAME               "gBmpsMaxListenInterval"
#define CFG_BMPS_MAXIMUM_LI_MIN                ( 1 )
#define CFG_BMPS_MAXIMUM_LI_MAX                ( 65535 )
#define CFG_BMPS_MAXIMUM_LI_DEFAULT            ( 1 )

// gEnableAutoBmpsTimer has been previously published as an externally
// configurable parameter. See analysis of CR 178211 for detailed info
// on why we want to *always* set this to 1 i.e. we no longer want
// this parameter to be configurable. the clean solution would be for 
// users to not define this item in winreg so that the default value 
// (which needs to be changed to 1) gets picked up but we cannot rely on that 
// since this item has been published already hence the proposed
// solution to change the name of the item along with the change in the
// default value. also we could decide to not read this item from registry
// but leaving open the option of being able to configure this item for
// ASW's internal use
#define CFG_ENABLE_AUTO_BMPS_TIMER_NAME        "gEnableAutoBmpsTimer_INTERNAL"
#define CFG_ENABLE_AUTO_BMPS_TIMER_MIN         ( 0 )
#define CFG_ENABLE_AUTO_BMPS_TIMER_MAX         ( 1 )
#define CFG_ENABLE_AUTO_BMPS_TIMER_DEFAULT     ( 1 )

#define CFG_AUTO_BMPS_TIMER_VALUE_NAME         "gAutoBmpsTimerValue" 
#define CFG_AUTO_BMPS_TIMER_VALUE_MIN          ( 1000 )
#define CFG_AUTO_BMPS_TIMER_VALUE_MAX          ( 4294967295UL )
#define CFG_AUTO_BMPS_TIMER_VALUE_DEFAULT      ( 1000 )        
    
#define CFG_MAX_RX_AMPDU_FACTOR_NAME           "gMaxRxAmpduFactor"   
#define CFG_MAX_RX_AMPDU_FACTOR_MIN            WNI_CFG_MAX_RX_AMPDU_FACTOR_STAMIN 
#define CFG_MAX_RX_AMPDU_FACTOR_MAX            WNI_CFG_MAX_RX_AMPDU_FACTOR_STAMAX 
#define CFG_MAX_RX_AMPDU_FACTOR_DEFAULT        WNI_CFG_MAX_RX_AMPDU_FACTOR_STADEF 

typedef enum
{
    eHDD_DOT11_MODE_AUTO = 0, //covers all things we support
    eHDD_DOT11_MODE_abg,      //11a/b/g only, no HT, no proprietary
    eHDD_DOT11_MODE_11b,
    eHDD_DOT11_MODE_11g,
    eHDD_DOT11_MODE_11n,
    eHDD_DOT11_MODE_11g_ONLY,
    eHDD_DOT11_MODE_11n_ONLY,
    eHDD_DOT11_MODE_11b_ONLY,
}eHddDot11Mode;

#define CFG_DOT11_MODE_NAME                    "gDot11Mode"
#define CFG_DOT11_MODE_MIN                     eHDD_DOT11_MODE_AUTO
#define CFG_DOT11_MODE_MAX                     eHDD_DOT11_MODE_11b_ONLY
#define CFG_DOT11_MODE_DEFAULT                 eHDD_DOT11_MODE_11n

#define CFG_CHANNEL_BONDING_MODE_NAME          "gChannelBondingMode"
#define CFG_CHANNEL_BONDING_MODE_MIN           WNI_CFG_CHANNEL_BONDING_MODE_STAMIN 
#define CFG_CHANNEL_BONDING_MODE_MAX           WNI_CFG_CHANNEL_BONDING_MODE_STAMAX 
#define CFG_CHANNEL_BONDING_MODE_DEFAULT       WNI_CFG_CHANNEL_BONDING_MODE_STADEF 

#define CFG_FIXED_RATE_NAME                    "gFixedRate"
#define CFG_FIXED_RATE_MIN                     WNI_CFG_FIXED_RATE_STAMIN
#define CFG_FIXED_RATE_MAX                     WNI_CFG_FIXED_RATE_STAMAX
#define CFG_FIXED_RATE_DEFAULT                 WNI_CFG_FIXED_RATE_STADEF 
 
#define CFG_SHORT_GI_20MHZ_NAME                "gShortGI20Mhz"
#define CFG_SHORT_GI_20MHZ_MIN                 WNI_CFG_SHORT_GI_20MHZ_STAMIN
#define CFG_SHORT_GI_20MHZ_MAX                 WNI_CFG_SHORT_GI_20MHZ_STAMAX 
#define CFG_SHORT_GI_20MHZ_DEFAULT             WNI_CFG_SHORT_GI_20MHZ_STADEF 

#define CFG_BLOCK_ACK_AUTO_SETUP_NAME          "gBlockAckAutoSetup"
#define CFG_BLOCK_ACK_AUTO_SETUP_MIN           ( 0 )
#define CFG_BLOCK_ACK_AUTO_SETUP_MAX           ( 1 )
#define CFG_BLOCK_ACK_AUTO_SETUP_DEFAULT       ( 1 )

#define CFG_SCAN_RESULT_AGE_COUNT_NAME         "gScanResultAgeCount"
#define CFG_SCAN_RESULT_AGE_COUNT_MIN          ( 2 )
#define CFG_SCAN_RESULT_AGE_COUNT_MAX          ( 100 )
#define CFG_SCAN_RESULT_AGE_COUNT_DEFAULT      ( 3 )

//All in seconds
//Not Connect, No Power Save
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_NAME    "gScanResultAgeNCNPS"
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_MIN     ( 10 )
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_MAX     ( 10000 )
#define CFG_SCAN_RESULT_AGE_TIME_NCNPS_DEFAULT ( 50 )
//Not Connect, Power Save
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_NAME     "gScanResultAgeNCPS"
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_MIN      ( 10 )
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_MAX      ( 10000 )
#define CFG_SCAN_RESULT_AGE_TIME_NCPS_DEFAULT  ( 300 )
//Connect, No Power Save
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_NAME     "gScanResultAgeCNPS"
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_MIN      ( 10 )
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_MAX      ( 10000 )
#define CFG_SCAN_RESULT_AGE_TIME_CNPS_DEFAULT  ( 150 )
//Connect, Power Save
#define CFG_SCAN_RESULT_AGE_TIME_CPS_NAME      "gScanResultAgeCPS"
#define CFG_SCAN_RESULT_AGE_TIME_CPS_MIN       ( 10 )
#define CFG_SCAN_RESULT_AGE_TIME_CPS_MAX       ( 10000 )
#define CFG_SCAN_RESULT_AGE_TIME_CPS_DEFAULT   ( 600 )

#define CFG_RSSI_CATEGORY_GAP_NAME             "gRssiCatGap"
#define CFG_RSSI_CATEGORY_GAP_MIN              ( 10 )  
#define CFG_RSSI_CATEGORY_GAP_MAX              ( 100 )  
#define CFG_RSSI_CATEGORY_GAP_DEFAULT          ( 30 )

#define CFG_STAT_TIMER_INTERVAL_NAME           "gStatTimerInterval"
#define CFG_STAT_TIMER_INTERVAL_MIN            ( 50 )     //ms
#define CFG_STAT_TIMER_INTERVAL_MAX            ( 10000 )  
#define CFG_STAT_TIMER_INTERVAL_DEFAULT        ( 500 )

#define CFG_SHORT_PREAMBLE_NAME                "gShortPreamble"
#define CFG_SHORT_PREAMBLE_MIN                 WNI_CFG_SHORT_PREAMBLE_STAMIN
#define CFG_SHORT_PREAMBLE_MAX                 WNI_CFG_SHORT_PREAMBLE_STAMAX
#define CFG_SHORT_PREAMBLE_DEFAULT             WNI_CFG_SHORT_PREAMBLE_STADEF

#define CFG_IBSS_AUTO_BSSID_NAME               "gAutoIbssBssid"
#define CFG_IBSS_AUTO_BSSID_MIN                WNI_CFG_IBSS_AUTO_BSSID_STAMIN
#define CFG_IBSS_AUTO_BSSID_MAX                WNI_CFG_IBSS_AUTO_BSSID_STAMAX
#define CFG_IBSS_AUTO_BSSID_DEFAULT            WNI_CFG_IBSS_AUTO_BSSID_STADEF

#define CFG_IBSS_BSSID_NAME                    "gIbssBssid"
#define CFG_IBSS_BSSID_MIN                     "000000000000"
#define CFG_IBSS_BSSID_MAX                     "ffffffffffff"
#define CFG_IBSS_BSSID_DEFAULT                 "000AF5040506"

#define CFG_INTF0_MAC_ADDR_NAME                  "Intf0MacAddress"
#define CFG_INTF0_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF0_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF0_MAC_ADDR_DEFAULT               "000AF5898980"

#define CFG_INTF1_MAC_ADDR_NAME                  "Intf1MacAddress"
#define CFG_INTF1_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF1_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF1_MAC_ADDR_DEFAULT               "000AF5898981"

#define CFG_INTF2_MAC_ADDR_NAME                  "Intf2MacAddress"
#define CFG_INTF2_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF2_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF2_MAC_ADDR_DEFAULT               "000AF5898982"

#define CFG_INTF3_MAC_ADDR_NAME                  "Intf3MacAddress"
#define CFG_INTF3_MAC_ADDR_MIN                   "000000000000"
#define CFG_INTF3_MAC_ADDR_MAX                   "ffffffffffff"
#define CFG_INTF3_MAC_ADDR_DEFAULT               "000AF5898983"

#ifdef WLAN_SOFTAP_FEATURE
#define CFG_AP_QOS_UAPSD_MODE_NAME             "gEnableApUapsd" // ACs to setup U-APSD for at assoc
#define CFG_AP_QOS_UAPSD_MODE_MIN              ( 0 )
#define CFG_AP_QOS_UAPSD_MODE_MAX              ( 1 ) 
#define CFG_AP_QOS_UAPSD_MODE_DEFAULT          ( 1 )   

#define CFG_AP_COUNTRY_CODE                    "gAPCntryCode"
#define CFG_AP_COUNTRY_CODE_MIN                "USI"
#define CFG_AP_COUNTRY_CODE_MAX                "USI"
#define CFG_AP_COUNTRY_CODE_DEFAULT            "FFF"

#define CFG_AP_ENABLE_PROTECTION_MODE_NAME            "gEnableApProt"
#define CFG_AP_ENABLE_PROTECTION_MODE_MIN             ( 0 )
#define CFG_AP_ENABLE_PROTECTION_MODE_MAX             ( 1 )
#define CFG_AP_ENABLE_PROTECTION_MODE_DEFAULT         ( 1 )

// Bit map for CFG_AP_PROTECTION_MODE_DEFAULT
// LOWER byte for associated stations
// UPPER byte for overlapping stations
// each byte will have the following info
// bit15 bit14 bit13     bit12  bit11 bit10    bit9     bit8
// OBSS  RIFS  LSIG_TXOP NON_GF HT20  FROM_11G FROM_11B FROM_11A
// bit7  bit6  bit5      bit4   bit3  bit2     bit1     bit0
// OBSS  RIFS  LSIG_TXOP NON_GF HT_20 FROM_11G FROM_11B FROM_11A
#define CFG_AP_PROTECTION_MODE_NAME            "gApProtection"
#define CFG_AP_PROTECTION_MODE_MIN             ( 0x0 )
#define CFG_AP_PROTECTION_MODE_MAX             ( 0xFFFF )
#define CFG_AP_PROTECTION_MODE_DEFAULT         ( 0xBFFF )

#define CFG_AP_OBSS_PROTECTION_MODE_NAME       "gEnableApOBSSProt" 
#define CFG_AP_OBSS_PROTECTION_MODE_MIN        ( 0 )
#define CFG_AP_OBSS_PROTECTION_MODE_MAX        ( 1 ) 
#define CFG_AP_OBSS_PROTECTION_MODE_DEFAULT    ( 0 )   

#define CFG_AP_STA_SECURITY_SEPERATION_NAME    "gDisableIntraBssFwd"
#define CFG_AP_STA_SECURITY_SEPERATION_MIN     ( 0 )
#define CFG_AP_STA_SECURITY_SEPERATION_MAX     ( 1 ) 
#define CFG_AP_STA_SECURITY_SEPERATION_DEFAULT ( 0 )   

#define CFG_AP_LISTEN_MODE_NAME               "gEnablePhyAgcListenMode" 
#define CFG_AP_LISTEN_MODE_MIN                (0)
#define CFG_AP_LISTEN_MODE_MAX                (128) 
#define CFG_AP_LISTEN_MODE_DEFAULT            (128)   

#define CFG_AP_AUTO_SHUT_OFF                "gAPAutoShutOff"
#define CFG_AP_AUTO_SHUT_OFF_MIN            ( 0 )
#define CFG_AP_AUTO_SHUT_OFF_MAX            ( 4294967295UL )
#define CFG_AP_AUTO_SHUT_OFF_DEFAULT        ( 0 )

#define CFG_FRAMES_PROCESSING_TH_MODE_NAME     "gMinFramesProcThres"
#define CFG_FRAMES_PROCESSING_TH_MIN           ( 0 )
#define CFG_FRAMES_PROCESSING_TH_MAX           ( 39 )
#define CFG_FRAMES_PROCESSING_TH_DEFAULT       ( 0 )

#endif

#define CFG_BEACON_INTERVAL_NAME               "gBeaconInterval"
#define CFG_BEACON_INTERVAL_MIN                WNI_CFG_BEACON_INTERVAL_STAMIN
#define CFG_BEACON_INTERVAL_MAX                WNI_CFG_BEACON_INTERVAL_STAMAX
#define CFG_BEACON_INTERVAL_DEFAULT            WNI_CFG_BEACON_INTERVAL_STADEF

//Handoff Configuration Parameters
#define CFG_ENABLE_HANDOFF_NAME                "gEnableHandoff"
#define CFG_ENABLE_HANDOFF_MIN                 ( 0 )
#define CFG_ENABLE_HANDOFF_MAX                 ( 1 )
#define CFG_ENABLE_HANDOFF_DEFAULT             ( 1 )

#ifdef FEATURE_WLAN_GEN6_ROAMING
//Handoff : No Wifi Params
#define CFG_RSSI_FILTER_CONST_NO_WIFI_NAME     "gRssiFilterConstNoWifi"
#define CFG_RSSI_FILTER_CONST_NO_WIFI_MIN      ( 0 )
#define CFG_RSSI_FILTER_CONST_NO_WIFI_MAX      ( 100 )
#define CFG_RSSI_FILTER_CONST_NO_WIFI_DEFAULT  ( 90 )

#define CFG_CHANNEL_SCAN_TIME_NAME             "gChannelScanTime"
#define CFG_CHANNEL_SCAN_TIME_MIN              ( 0 )
#define CFG_CHANNEL_SCAN_TIME_MAX              ( 4294967295UL )
#define CFG_CHANNEL_SCAN_TIME_DEFAULT          ( 120 )

#define CFG_RSSI_THRESH_NEIGH_NAME             "gRssiThreshNeigh"
#define CFG_RSSI_THRESH_NEIGH_MIN              ( 0 )
#define CFG_RSSI_THRESH_NEIGH_MAX              ( 100 )
#define CFG_RSSI_THRESH_NEIGH_DEFAULT          ( 95 )

#define CFG_RSSI_THRESH_ASSOC_NAME             "gRssiThreshAssoc"
#define CFG_RSSI_THRESH_ASSOC_MIN              ( 0 )
#define CFG_RSSI_THRESH_ASSOC_MAX              ( 100 )
#define CFG_RSSI_THRESH_ASSOC_DEFAULT          ( 80 )

#define CFG_ACTIVE_SCAN_INTERVAL_NAME          "gActiveScanInterval"
#define CFG_ACTIVE_SCAN_INTERVAL_MIN           ( 0 )
#define CFG_ACTIVE_SCAN_INTERVAL_MAX           ( 4294967295UL )
#define CFG_ACTIVE_SCAN_INTERVAL_DEFAULT       ( 30000 )

#define CFG_ACTIVE_SCAN_DURATION_NAME          "gActiveDuration"
#define CFG_ACTIVE_SCAN_DURATION_MIN           ( 0 )
#define CFG_ACTIVE_SCAN_DURATION_MAX           ( 4294967295UL )
#define CFG_ACTIVE_SCAN_DURATION_DEFAULT       ( 120000 )

//Handoff : No Traffic Params
#define CFG_RSSI_FILTER_CONST_NT_NAME          "gRssiFilterConstNt"
#define CFG_RSSI_FILTER_CONST_NT_MIN           ( 0 )
#define CFG_RSSI_FILTER_CONST_NT_MAX           ( 100 )
#define CFG_RSSI_FILTER_CONST_NT_DEFAULT       ( 90 )

#define CFG_NUM_CANDIDATE_SET_NT_NAME          "gNumCandidateSetNt"
#define CFG_NUM_CANDIDATE_SET_NT_MIN           ( 1 )
#define CFG_NUM_CANDIDATE_SET_NT_MAX           ( 20 )
#define CFG_NUM_CANDIDATE_SET_NT_DEFAULT       ( 5 )

#define CFG_INACT_THRESH_NT_NAME               "gInactThreshNt"
#define CFG_INACT_THRESH_NT_MIN                ( 1 )
#define CFG_INACT_THRESH_NT_MAX                ( 1000 )
#define CFG_INACT_THRESH_NT_DEFAULT            ( 25 )

#define CFG_INACT_PERIOD_NT_NAME                "gInactPeriodNt"
#define CFG_INACT_PERIOD_NT_MIN                 ( 0 )
#define CFG_INACT_PERIOD_NT_MAX                 ( 4294967295UL )
#define CFG_INACT_PERIOD_NT_DEFAULT             ( 2000 )

#define CFG_BEST_CANDT_AP_RSSI_DELTA_NAME       "gBestCandidateApRssiDeltaNt"
#define CFG_BEST_CANDT_AP_RSSI_DELTA_MIN        ( 0 )
#define CFG_BEST_CANDT_AP_RSSI_DELTA_MAX        ( 100 )
#define CFG_BEST_CANDT_AP_RSSI_DELTA_DEFAULT    ( 10 )

#define CFG_NEIGH_AP_BG_SCAN_INTERVAL_NAME     "gNeighApBgScanIntervalNt"
#define CFG_NEIGH_AP_BG_SCAN_INTERVAL_MIN      ( 0 )
#define CFG_NEIGH_AP_BG_SCAN_INTERVAL_MAX      ( 4294967295UL )
#define CFG_NEIGH_AP_BG_SCAN_INTERVAL_DEFAULT  ( 120000 )

#define CFG_NEIGH_AP_INCR_NT_NAME              "gNeighApIncrNt"
#define CFG_NEIGH_AP_INCR_NT_MIN               ( 0 )
#define CFG_NEIGH_AP_INCR_NT_MAX               ( 4294967295UL )
#define CFG_NEIGH_AP_INCR_NT_DEFAULT           ( 240000 )
  
#define CFG_RSSI_THRESH_CANDT_NT_NAME          "gRssiThreshCdtNt"
#define CFG_RSSI_THRESH_CANDT_NT_MIN           ( 0 )
#define CFG_RSSI_THRESH_CANDT_NT_MAX           ( 100 )
#define CFG_RSSI_THRESH_CANDT_NT_DEFAULT       ( 75 )

#define CFG_PMK_THRESH_RSSI_NT_NAME            "gPmkThreshRssiNt"
#define CFG_PMK_THRESH_RSSI_NT_MIN             ( 0 )
#define CFG_PMK_THRESH_RSSI_NT_MAX             ( 100 )
#define CFG_PMK_THRESH_RSSI_NT_DEFAULT         ( 3 )

#define CFG_RSSI_THRESH_CURR_AP_NT_NAME        "gRssiThreshCurrApNt"
#define CFG_RSSI_THRESH_CURR_AP_NT_MIN         ( 0 )
#define CFG_RSSI_THRESH_CURR_AP_NT_MAX         ( 100 )
#define CFG_RSSI_THRESH_CURR_AP_NT_DEFAULT     ( 50 )

// Handoff : Non Real Time Params
#define CFG_RSSI_FILTER_CONST_NRT_NAME         "gRssiFilterConstNrt"
#define CFG_RSSI_FILTER_CONST_NRT_MIN          ( 0 )
#define CFG_RSSI_FILTER_CONST_NRT_MAX          ( 100 )
#define CFG_RSSI_FILTER_CONST_NRT_DEFAULT      ( 90 )

#define CFG_NUM_CANDIDATE_SET_NRT_NAME         "gNumCandidateSetNrt"
#define CFG_NUM_CANDIDATE_SET_NRT_MIN          ( 1 )
#define CFG_NUM_CANDIDATE_SET_NRT_MAX          ( 20 )
#define CFG_NUM_CANDIDATE_SET_NRT_DEFAULT      ( 5 )

#define CFG_RSSI_THRESH_CURR_AP_NRT_NAME       "gRssiThreshCurrApNrt"
#define CFG_RSSI_THRESH_CURR_AP_NRT_MIN        ( 0 )
#define CFG_RSSI_THRESH_CURR_AP_NRT_MAX        ( 100 )
#define CFG_RSSI_THRESH_CURR_AP_NRT_DEFAULT    ( 50 )

#define CFG_RSSI_THRESH_EMPTY_NRT_NAME         "gRssiThreshEmptyNrt"
#define CFG_RSSI_THRESH_EMPTY_NRT_MIN          ( 0 )
#define CFG_RSSI_THRESH_EMPTY_NRT_MAX          ( 100 )
#define CFG_RSSI_THRESH_EMPTY_NRT_DEFAULT      ( 75 )

#define CFG_RSSI_THRESH_HO_NRT_NAME            "gRssiThreshHoNrt"
#define CFG_RSSI_THRESH_HO_NRT_MIN             ( 0 )
#define CFG_RSSI_THRESH_HO_NRT_MAX             ( 100 )
#define CFG_RSSI_THRESH_HO_NRT_DEFAULT         ( 80 )

#define CFG_RSSI_THRESH_CDT_SET_NRT_NAME       "gRssiThreshCdtSetNrt"
#define CFG_RSSI_THRESH_CDT_SET_NRT_MIN        ( 0 )
#define CFG_RSSI_THRESH_CDT_SET_NRT_MAX        ( 100 )
#define CFG_RSSI_THRESH_CDT_SET_NRT_DEFAULT    ( 75 )
  
#define CFG_BG_SCAN_INTERVAL_NRT_NAME          "gBgScanIntervalNrt"
#define CFG_BG_SCAN_INTERVAL_NRT_MIN           ( 0 )
#define CFG_BG_SCAN_INTERVAL_NRT_MAX           ( 4294967295UL )
#define CFG_BG_SCAN_INTERVAL_NRT_DEFAULT       ( 600 )

#define CFG_BG_SCAN_INCR_INTERVAL_NRT_NAME     "gBgScanIncrIntervalNrt"
#define CFG_BG_SCAN_INCR_INTERVAL_NRT_MIN      ( 0 )
#define CFG_BG_SCAN_INCR_INTERVAL_NRT_MAX      ( 4294967295UL )
#define CFG_BG_SCAN_INCR_INTERVAL_NRT_DEFAULT  ( 1200 )

#define CFG_BG_SCAN_DELAY_INTERVAL_NRT_NAME    "gBgScanDelayIntervalNrt"
#define CFG_BG_SCAN_DELAY_INTERVAL_NRT_MIN     ( 0 )
#define CFG_BG_SCAN_DELAY_INTERVAL_NRT_MAX     ( 4294967295UL )
#define CFG_BG_SCAN_DELAY_INTERVAL_NRT_DEFAULT ( 120000 )

#define CFG_PER_MSMT_INTERVAL_NRT_NAME         "gPerMsmtIntervalNrt"
#define CFG_PER_MSMT_INTERVAL_NRT_MIN          ( 0 )
#define CFG_PER_MSMT_INTERVAL_NRT_MAX          ( 4294967295UL )
#define CFG_PER_MSMT_INTERVAL_NRT_DEFAULT      ( 2000 )

#define CFG_HO_FROM_CURR_AP_NRT_NAME           "gPerThresholdHoNrt"
#define CFG_HO_FROM_CURR_AP_NRT_MIN            ( 0 )
#define CFG_HO_FROM_CURR_AP_NRT_MAX            ( 100 )
#define CFG_HO_FROM_CURR_AP_NRT_DEFAULT        ( 5 )
  
#define CFG_PMK_CACHE_RSSI_DELTA_NRT_NAME      "gPmkCacheRssiDeltaNrt"
#define CFG_PMK_CACHE_RSSI_DELTA_NRT_MIN       ( 0 )
#define CFG_PMK_CACHE_RSSI_DELTA_NRT_MAX       ( 100 )
#define CFG_PMK_CACHE_RSSI_DELTA_NRT_DEFAULT   ( 3 )
  
#define CFG_BEST_CDT_AP_RSSI_DELTA_NRT_NAME    "gBestCdtApRssiDeltaNrt"
#define CFG_BEST_CDT_AP_RSSI_DELTA_NRT_MIN     ( 0 )
#define CFG_BEST_CDT_AP_RSSI_DELTA_NRT_MAX     ( 100 )
#define CFG_BEST_CDT_AP_RSSI_DELTA_NRT_DEFAULT ( 10 )

//Handoff : Real Time Params
#define CFG_RSSI_FILTER_CONST_RT_NAME          "gRssiFilterConstRt" 
#define CFG_RSSI_FILTER_CONST_RT_MIN           ( 0 )
#define CFG_RSSI_FILTER_CONST_RT_MAX           ( 100 )
#define CFG_RSSI_FILTER_CONST_RT_DEFAULT       ( 90 )

#define CFG_NUM_CANDIDATE_SET_RT_NAME          "gNumCandidateSetRt"
#define CFG_NUM_CANDIDATE_SET_RT_MIN           ( 1 )
#define CFG_NUM_CANDIDATE_SET_RT_MAX           ( 20 )
#define CFG_NUM_CANDIDATE_SET_RT_DEFAULT       ( 5 )

#define CFG_RSSI_THRESH_CURR_AP_RT_NAME       "gRssiThreshCurrApRt"
#define CFG_RSSI_THRESH_CURR_AP_RT_MIN        ( 0 )
#define CFG_RSSI_THRESH_CURR_AP_RT_MAX        ( 100 )
#define CFG_RSSI_THRESH_CURR_AP_RT_DEFAULT    ( 50 )

#define CFG_RSSI_THRESH_HO_RT_NAME            "gRssiThreshHoRt"
#define CFG_RSSI_THRESH_HO_RT_MIN             ( 0 )
#define CFG_RSSI_THRESH_HO_RT_MAX             ( 100 )
#define CFG_RSSI_THRESH_HO_RT_DEFAULT         ( 80 )

#define CFG_RSSI_THRESH_CDT_SET_RT_NAME       "gRssiThreshCdtSetRt"
#define CFG_RSSI_THRESH_CDT_SET_RT_MIN        ( 0 )
#define CFG_RSSI_THRESH_CDT_SET_RT_MAX        ( 100 )
#define CFG_RSSI_THRESH_CDT_SET_RT_DEFAULT    ( 75 )
 
#define CFG_BG_SCAN_INTERVAL_RT_NAME           "gBgScanIntervalRt"
#define CFG_BG_SCAN_INTERVAL_RT_MIN            ( 0 )
#define CFG_BG_SCAN_INTERVAL_RT_MAX            ( 4294967295UL )
#define CFG_BG_SCAN_INTERVAL_RT_DEFAULT        ( 300 )

#define CFG_PER_MSMT_INTERVAL_RT_NAME          "gPerMsmtIntervalRt"
#define CFG_PER_MSMT_INTERVAL_RT_MIN           ( 0 )
#define CFG_PER_MSMT_INTERVAL_RT_MAX           ( 4294967295UL )
#define CFG_PER_MSMT_INTERVAL_RT_DEFAULT       ( 2000 )

#define CFG_HO_FROM_CURR_AP_RT_NAME            "gPerThresholdHoRt"
#define CFG_HO_FROM_CURR_AP_RT_MIN             ( 0 )
#define CFG_HO_FROM_CURR_AP_RT_MAX             ( 100 )
#define CFG_HO_FROM_CURR_AP_RT_DEFAULT         ( 10 )
  
#define CFG_PMK_CACHE_RSSI_DELTA_RT_NAME       "gPmkCacheRssiDeltaRt"
#define CFG_PMK_CACHE_RSSI_DELTA_RT_MIN        ( 0 )
#define CFG_PMK_CACHE_RSSI_DELTA_RT_MAX        ( 100 )
#define CFG_PMK_CACHE_RSSI_DELTA_RT_DEFAULT    ( 3 )
  
#define CFG_BEST_CDT_AP_RSSI_DELTA_RT_NAME    "gBestCdtApRssiDeltaRt"
#define CFG_BEST_CDT_AP_RSSI_DELTA_RT_MIN     ( 0 )
#define CFG_BEST_CDT_AP_RSSI_DELTA_RT_MAX     ( 100 )
#define CFG_BEST_CDT_AP_RSSI_DELTA_RT_DEFAULT ( 10 )

#endif //#ifdef FEATURE_WLAN_GEN6_ROAMING

//Additional Handoff related Parameters  
#define CFG_ENABLE_IDLE_SCAN_NAME             "gEnableIdleScan"      
#define CFG_ENABLE_IDLE_SCAN_MIN              ( 0 )
#define CFG_ENABLE_IDLE_SCAN_MAX              ( 1 )
#define CFG_ENABLE_IDLE_SCAN_DEFAULT          ( 1 ) 

#define CFG_ROAMING_TIME_NAME                 "gRoamingTime"
#define CFG_ROAMING_TIME_MIN                  ( 0 )
#define CFG_ROAMING_TIME_MAX                  ( 4294967UL )
#define CFG_ROAMING_TIME_DEFAULT              ( 10 )

#define CFG_VCC_RSSI_TRIGGER_NAME             "gVccRssiTrigger"
#define CFG_VCC_RSSI_TRIGGER_MIN              ( 0 )
#define CFG_VCC_RSSI_TRIGGER_MAX              ( 80 ) 
#define CFG_VCC_RSSI_TRIGGER_DEFAULT          ( 80 )

#define CFG_VCC_UL_MAC_LOSS_THRESH_NAME       "gVccUlMacLossThresh"
#define CFG_VCC_UL_MAC_LOSS_THRESH_MIN        ( 0 )  
#define CFG_VCC_UL_MAC_LOSS_THRESH_MAX        ( 9 )
#define CFG_VCC_UL_MAC_LOSS_THRESH_DEFAULT    ( 9 )
   
#define CFG_PASSIVE_MAX_CHANNEL_TIME_NAME      "gPassiveMaxChannelTime"
#define CFG_PASSIVE_MAX_CHANNEL_TIME_MIN       ( 0 )
#define CFG_PASSIVE_MAX_CHANNEL_TIME_MAX       ( 10000 )
#define CFG_PASSIVE_MAX_CHANNEL_TIME_DEFAULT   ( 110 )

#define CFG_PASSIVE_MIN_CHANNEL_TIME_NAME      "gPassiveMinChannelTime"
#define CFG_PASSIVE_MIN_CHANNEL_TIME_MIN       ( 0 )
#define CFG_PASSIVE_MIN_CHANNEL_TIME_MAX       ( 10000 )
#define CFG_PASSIVE_MIN_CHANNEL_TIME_DEFAULT   ( 60 )

#define CFG_ACTIVE_MAX_CHANNEL_TIME_NAME       "gActiveMaxChannelTime"
#define CFG_ACTIVE_MAX_CHANNEL_TIME_MIN        ( 0 )
#define CFG_ACTIVE_MAX_CHANNEL_TIME_MAX        ( 10000 )
#define CFG_ACTIVE_MAX_CHANNEL_TIME_DEFAULT    ( 40 )

#define CFG_ACTIVE_MIN_CHANNEL_TIME_NAME       "gActiveMinChannelTime"
#define CFG_ACTIVE_MIN_CHANNEL_TIME_MIN        ( 0 )
#define CFG_ACTIVE_MIN_CHANNEL_TIME_MAX        ( 10000 )
#define CFG_ACTIVE_MIN_CHANNEL_TIME_DEFAULT    ( 20 )

#define CFG_MAX_PS_POLL_NAME                   "gMaxPsPoll"
#define CFG_MAX_PS_POLL_MIN                    WNI_CFG_MAX_PS_POLL_STAMIN
#define CFG_MAX_PS_POLL_MAX                    WNI_CFG_MAX_PS_POLL_STAMAX
#define CFG_MAX_PS_POLL_DEFAULT                WNI_CFG_MAX_PS_POLL_STADEF

#define CFG_MAX_TX_POWER_NAME                   "gTxPowerCap"
#define CFG_MAX_TX_POWER_MIN                    WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMIN
#define CFG_MAX_TX_POWER_MAX                    WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX
//Not to use CFG default because if no registry setting, this is ignored by SME.
#define CFG_MAX_TX_POWER_DEFAULT                WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX


#define CFG_LOW_GAIN_OVERRIDE_NAME             "gLowGainOverride"
#define CFG_LOW_GAIN_OVERRIDE_MIN              WNI_CFG_LOW_GAIN_OVERRIDE_STAMIN
#define CFG_LOW_GAIN_OVERRIDE_MAX              WNI_CFG_LOW_GAIN_OVERRIDE_STAMAX
#define CFG_LOW_GAIN_OVERRIDE_DEFAULT          WNI_CFG_LOW_GAIN_OVERRIDE_STADEF

#define CFG_RSSI_FILTER_PERIOD_NAME            "gRssiFilterPeriod"
#define CFG_RSSI_FILTER_PERIOD_MIN             WNI_CFG_RSSI_FILTER_PERIOD_STAMIN
#define CFG_RSSI_FILTER_PERIOD_MAX             WNI_CFG_RSSI_FILTER_PERIOD_STAMAX
// Increased this value for Non-CCX AP. This is cause FW RSSI Monitoring
// the consumer of this value is ON by default. So to impact power numbers
// we are setting this to a high value.
#define CFG_RSSI_FILTER_PERIOD_DEFAULT         WNI_CFG_RSSI_FILTER_PERIOD_STADEF

#define CFG_IGNORE_DTIM_NAME                   "gIgnoreDtim"
#define CFG_IGNORE_DTIM_MIN                    WNI_CFG_IGNORE_DTIM_STAMIN
#define CFG_IGNORE_DTIM_MAX                    WNI_CFG_IGNORE_DTIM_STAMAX
#define CFG_IGNORE_DTIM_DEFAULT                WNI_CFG_IGNORE_DTIM_STADEF

#define CFG_RX_ANT_CONFIGURATION_NAME          "gNumRxAnt"
#define CFG_RX_ANT_CONFIGURATION_NAME_MIN      ( 1 )
#define CFG_RX_ANT_CONFIGURATION_NAME_MAX      ( 2 )
#define CFG_RX_ANT_CONFIGURATION_NAME_DEFAULT  ( 2 )

#define CFG_FW_HEART_BEAT_MONITORING_NAME      "gEnableFWHeartBeatMonitoring"
#define CFG_FW_HEART_BEAT_MONITORING_MIN       ( 0 )
#define CFG_FW_HEART_BEAT_MONITORING_MAX       ( 1 )
#define CFG_FW_HEART_BEAT_MONITORING_DEFAULT   ( 1 )

#define CFG_FW_BEACON_FILTERING_NAME           "gEnableFWBeaconFiltering"
#define CFG_FW_BEACON_FILTERING_MIN            ( 0 )
#define CFG_FW_BEACON_FILTERING_MAX            ( 1 )
#define CFG_FW_BEACON_FILTERING_DEFAULT        ( 1 )

#define CFG_FW_RSSI_MONITORING_NAME            "gEnableFWRssiMonitoring"
#define CFG_FW_RSSI_MONITORING_MIN             ( 0 )
#define CFG_FW_RSSI_MONITORING_MAX             ( 1 )
#define CFG_FW_RSSI_MONITORING_DEFAULT         WNI_CFG_PS_ENABLE_RSSI_MONITOR_STADEF

#define CFG_DATA_INACTIVITY_TIMEOUT_NAME       "gDataInactivityTimeout"
#define CFG_DATA_INACTIVITY_TIMEOUT_MIN        ( 1 )
#define CFG_DATA_INACTIVITY_TIMEOUT_MAX        ( 255 )
#define CFG_DATA_INACTIVITY_TIMEOUT_DEFAULT    ( 20 )

#define CFG_NTH_BEACON_FILTER_NAME             "gNthBeaconFilter"
#define CFG_NTH_BEACON_FILTER_MIN              ( WNI_CFG_NTH_BEACON_FILTER_STAMIN )
#define CFG_NTH_BEACON_FILTER_MAX              ( WNI_CFG_NTH_BEACON_FILTER_STAMAX )
#define CFG_NTH_BEACON_FILTER_DEFAULT          ( WNI_CFG_NTH_BEACON_FILTER_STADEF )

#define CFG_RF_SETTLING_TIME_CLK_NAME          "rfSettlingTimeUs"
#define CFG_RF_SETTLING_TIME_CLK_MIN           ( 0 )
#define CFG_RF_SETTLING_TIME_CLK_MAX           ( 60000 )
#define CFG_RF_SETTLING_TIME_CLK_DEFAULT       ( 1500 )

#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_NAME          "gStaKeepAlivePeriod"
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MIN           ( 0 )
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_MAX           ( 65535)
#define CFG_INFRA_STA_KEEP_ALIVE_PERIOD_DEFAULT       ( 0 )

//WMM configuration
#define CFG_QOS_WMM_MODE_NAME                             "WmmIsEnabled"
#define CFG_QOS_WMM_MODE_MIN                               (0)
#define CFG_QOS_WMM_MODE_MAX                               (2) //HDD_WMM_NO_QOS
#define CFG_QOS_WMM_MODE_DEFAULT                           (0) //HDD_WMM_AUTO

#define CFG_QOS_WMM_80211E_ENABLED_NAME                   "80211eIsEnabled"
#define CFG_QOS_WMM_80211E_ENABLED_MIN                     (0)
#define CFG_QOS_WMM_80211E_ENABLED_MAX                     (1) 
#define CFG_QOS_WMM_80211E_ENABLED_DEFAULT                 (0) 

#define CFG_QOS_WMM_UAPSD_MASK_NAME                        "UapsdMask" // ACs to setup U-APSD for at assoc
#define CFG_QOS_WMM_UAPSD_MASK_MIN                         (0x00)
#define CFG_QOS_WMM_UAPSD_MASK_MAX                         (0xFF) 
#define CFG_QOS_WMM_UAPSD_MASK_DEFAULT                     (0x00)   

#define CFG_QOS_WMM_MAX_SP_LEN_NAME                        "MaxSpLength"
#define CFG_QOS_WMM_MAX_SP_LEN_MIN                          (0)
#define CFG_QOS_WMM_MAX_SP_LEN_MAX                          (3)
#define CFG_QOS_WMM_MAX_SP_LEN_DEFAULT                      (0)

#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_NAME           "InfraUapsdVoSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MAX             (4294967295UL )
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_DEFAULT         (20)

#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_NAME           "InfraUapsdVoSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MAX             (4294967295UL )
#define CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_DEFAULT         (2000)

#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_NAME           "InfraUapsdViSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MAX             (4294967295UL) 
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_DEFAULT         (300)

#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_NAME           "InfraUapsdViSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_DEFAULT         (2000)

#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_NAME           "InfraUapsdBeSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_DEFAULT         (300)

#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_NAME           "InfraUapsdBeSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_DEFAULT         (2000)

#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_NAME           "InfraUapsdBkSrvIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MAX             (4294967295UL)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_DEFAULT         (300)

#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_NAME           "InfraUapsdBkSuspIntv"
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MIN             (0)
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MAX             (4294967295UL)             
#define CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_DEFAULT         (2000)

#ifdef FEATURE_WLAN_CCX
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_NAME         "InfraInactivityInterval"
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MIN           (0)
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_MAX           (4294967295UL)
#define CFG_QOS_WMM_INFRA_INACTIVITY_INTERVAL_DEFAULT       (0) //disabled

#define CFG_CCX_FEATURE_ENABLED_NAME                       "CcxEnabled"
#define CFG_CCX_FEATURE_ENABLED_MIN                         (0)
#define CFG_CCX_FEATURE_ENABLED_MAX                         (1)
#define CFG_CCX_FEATURE_ENABLED_DEFAULT                     (0) //disabled
#endif // FEATURE_WLAN_CCX

#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX)
#define CFG_FT_RSSI_FILTER_PERIOD_NAME                     "FTRssiFilterPeriod"
#define CFG_FT_RSSI_FILTER_PERIOD_MIN                      WNI_CFG_FT_RSSI_FILTER_PERIOD_STAMIN
#define CFG_FT_RSSI_FILTER_PERIOD_MAX                      WNI_CFG_FT_RSSI_FILTER_PERIOD_STAMAX
#define CFG_FT_RSSI_FILTER_PERIOD_DEFAULT                  WNI_CFG_FT_RSSI_FILTER_PERIOD_STADEF 

// This flag will control fasttransition in case of 11r and ccx.
// Basically with this the whole neighbor roam, pre-auth, reassoc
// can be turned ON/OFF. 
// With this turned OFF 11r will completely not work.
// For 11r this flag has to be ON.
// For CCX fastroam will not work.
#define CFG_FAST_TRANSITION_ENABLED_NAME                    "FastTransitionEnabled"
#define CFG_FAST_TRANSITION_ENABLED_NAME_MIN                (0)
#define CFG_FAST_TRANSITION_ENABLED_NAME_MAX                (1)
#define CFG_FAST_TRANSITION_ENABLED_NAME_DEFAULT            (0) //disabled
#endif

#define CFG_QOS_WMM_PKT_CLASSIFY_BASIS_NAME                "PktClassificationBasis" // DSCP or 802.1Q
#define CFG_QOS_WMM_PKT_CLASSIFY_BASIS_MIN                  (0)
#define CFG_QOS_WMM_PKT_CLASSIFY_BASIS_MAX                  (1)
#define CFG_QOS_WMM_PKT_CLASSIFY_BASIS_DEFAULT              (0) //DSCP

/* default TSPEC parameters for AC_VO */
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_NAME                   "InfraDirAcVo"
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_VO_DEFAULT                 (3) //WLAN_QCT_CUST_WMM_TSDIR_BOTH

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_NAME         "InfraNomMsduSizeAcVo"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_DEFAULT       (0x80D0)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_NAME        "InfraMeanDataRateAcVo"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_DEFAULT      (0x14500)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_NAME          "InfraMinPhyRateAcVo"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_VO_NAME                   "InfraSbaAcVo"
#define CFG_QOS_WMM_INFRA_SBA_AC_VO_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_VO_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_VO_DEFAULT                 (0x2001)

/* default TSPEC parameters for AC_VI */
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_NAME                   "InfraDirAcVi"
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_VI_DEFAULT                 (3) //WLAN_QCT_CUST_WMM_TSDIR_BOTH

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_NAME         "InfraNomMsduSizeAcVi"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_DEFAULT       (0x85DC)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_NAME        "InfraMeanDataRateAcVi"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_DEFAULT      (0x57E40)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_NAME          "InfraMinPhyRateAcVi"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_VI_NAME                   "InfraSbaAcVi"
#define CFG_QOS_WMM_INFRA_SBA_AC_VI_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_VI_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_VI_DEFAULT                 (0x2001)

/* default TSPEC parameters for AC_BE*/
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_NAME                   "InfraDirAcBe"
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_BE_DEFAULT                 (3) //WLAN_QCT_CUST_WMM_TSDIR_BOTH

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_NAME         "InfraNomMsduSizeAcBe"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_DEFAULT       (0x85DC)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_NAME        "InfraMeanDataRateAcBe"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_DEFAULT      (0x493E0)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_NAME          "InfraMinPhyRateAcBe"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_BE_NAME                   "InfraSbaAcBe"
#define CFG_QOS_WMM_INFRA_SBA_AC_BE_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_BE_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_BE_DEFAULT                 (0x2001)

/* default TSPEC parameters for AC_Bk*/
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_NAME                   "InfraDirAcBk"
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_MIN                     (0)
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_MAX                     (3)
#define CFG_QOS_WMM_INFRA_DIR_AC_BK_DEFAULT                 (3) //WLAN_QCT_CUST_WMM_TSDIR_BOTH

#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_NAME         "InfraNomMsduSizeAcBk"
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MIN           (0x0)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MAX           (0xFFFF)
#define CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_DEFAULT       (0x85DC)

#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_NAME        "InfraMeanDataRateAcBk"
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MIN          (0x0)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MAX          (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_DEFAULT      (0x493E0)

#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_NAME          "InfraMinPhyRateAcBk"
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MIN            (0x0)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MAX            (0xFFFFFFFF)
#define CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_DEFAULT        (0x5B8D80)

#define CFG_QOS_WMM_INFRA_SBA_AC_BK_NAME                   "InfraSbaAcBk"
#define CFG_QOS_WMM_INFRA_SBA_AC_BK_MIN                     (0x2001)
#define CFG_QOS_WMM_INFRA_SBA_AC_BK_MAX                     (0xFFFF)
#define CFG_QOS_WMM_INFRA_SBA_AC_BK_DEFAULT                 (0x2001)

// TL configuration
#define CFG_TL_WFQ_BK_WEIGHT_NAME                           "WfqBkWeight"
#define CFG_TL_WFQ_BK_WEIGHT_MIN                            1
#define CFG_TL_WFQ_BK_WEIGHT_MAX                            0xFF
#define CFG_TL_WFQ_BK_WEIGHT_DEFAULT                        1

#define CFG_TL_WFQ_BE_WEIGHT_NAME                           "WfqBeWeight"
#define CFG_TL_WFQ_BE_WEIGHT_MIN                            1
#define CFG_TL_WFQ_BE_WEIGHT_MAX                            0xFF
#define CFG_TL_WFQ_BE_WEIGHT_DEFAULT                        3

#define CFG_TL_WFQ_VI_WEIGHT_NAME                           "WfqViWeight"
#define CFG_TL_WFQ_VI_WEIGHT_MIN                            1
#define CFG_TL_WFQ_VI_WEIGHT_MAX                            0xFF
#define CFG_TL_WFQ_VI_WEIGHT_DEFAULT                        5

#define CFG_TL_WFQ_VO_WEIGHT_NAME                           "WfqVoWeight"
#define CFG_TL_WFQ_VO_WEIGHT_MIN                            1
#define CFG_TL_WFQ_VO_WEIGHT_MAX                            0xFF
#define CFG_TL_WFQ_VO_WEIGHT_DEFAULT                        7

#define CFG_TL_DELAYED_TRGR_FRM_INT_NAME                   "DelayedTriggerFrmInt"
#define CFG_TL_DELAYED_TRGR_FRM_INT_MIN                     1
#define CFG_TL_DELAYED_TRGR_FRM_INT_MAX                     (4294967295UL)
#define CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT                 3000

#if defined WLAN_FEATURE_VOWIFI
#define CFG_RRM_ENABLE_NAME                              "gRrmEnable"
#define CFG_RRM_ENABLE_MIN                               (0)
#define CFG_RRM_ENABLE_MAX                               (1)  
#define CFG_RRM_ENABLE_DEFAULT                           (0)

#define CFG_RRM_OPERATING_CHAN_MAX_DURATION_NAME         "gRrmOperChanMax" //section 11.10.3 IEEE std. 802.11k-2008
#define CFG_RRM_OPERATING_CHAN_MAX_DURATION_MIN          (0)             //Maxduration = 2^(maxDuration - 4) * bcnIntvl.
#define CFG_RRM_OPERATING_CHAN_MAX_DURATION_MAX          (8)  
#define CFG_RRM_OPERATING_CHAN_MAX_DURATION_DEFAULT      (3)             //max duration = 2^-1 * bcnIntvl (50% of bcn intvl)

#define CFG_RRM_NON_OPERATING_CHAN_MAX_DURATION_NAME     "gRrmNonOperChanMax" //Same as above.
#define CFG_RRM_NON_OPERATING_CHAN_MAX_DURATION_MIN      (0)
#define CFG_RRM_NON_OPERATING_CHAN_MAX_DURATION_MAX      (8)  
#define CFG_RRM_NON_OPERATING_CHAN_MAX_DURATION_DEFAULT  (3)

#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_NAME            "gRrmRandnIntvl"
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_MIN             (10)
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_MAX             (100)  
#define CFG_RRM_MEAS_RANDOMIZATION_INTVL_DEFAULT         (100)
#endif

#define CFG_QOS_IMPLICIT_SETUP_ENABLED_NAME                 "ImplicitQosIsEnabled"
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_MIN                  (0)
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_MAX                  (1) 
#define CFG_QOS_IMPLICIT_SETUP_ENABLED_DEFAULT              (1)

#define CFG_19P2_MHZ_PMIC_CLK_ENABLED_NAME                  "19p2MhzPmicClkEnabled"
#define CFG_19P2_MHZ_PMIC_CLK_ENABLED_MIN                   (0)
#define CFG_19P2_MHZ_PMIC_CLK_ENABLED_MAX                   (1) 
#define CFG_19P2_MHZ_PMIC_CLK_ENABLED_DEFAULT               (0)

#define CFG_ENABLE_LOGP_NAME                                "gEnableLogp"
#define CFG_ENABLE_LOGP_MIN                                 ( 0 )
#define CFG_ENABLE_LOGP_MAX                                 ( 1 )
#define CFG_ENABLE_LOGP_DEFAULT                             ( 0 )

#define CFG_BTC_EXECUTION_MODE_NAME                         "BtcExecutionMode"
#define CFG_BTC_EXECUTION_MODE_MIN                          ( 0 )
#define CFG_BTC_EXECUTION_MODE_MAX                          ( 5 )
#define CFG_BTC_EXECUTION_MODE_DEFAULT                      ( 0 )

#define CFG_BTC_DHCP_PROTECTION_NAME                         "BtcConsBtSlotToBlockDuringDhcp"
#define CFG_BTC_DHCP_PROTECTION_MIN                          ( 0 )
#define CFG_BTC_DHCP_PROTECTION_MAX                          ( 0xFF )
#define CFG_BTC_DHCP_PROTECTION_DEFAULT                      ( 0 )

#define CFG_BTC_A2DP_DHCP_PROTECTION_NAME                    "BtcA2DPDhcpProtectLevel"
#define CFG_BTC_A2DP_DHCP_PROTECTION_MIN                     ( 0 )
#define CFG_BTC_A2DP_DHCP_PROTECTION_MAX                     ( 0xFF )
#define CFG_BTC_A2DP_DHCP_PROTECTION_DEFAULT                 ( 7 )

#if defined WLAN_FEATURE_VOWIFI_11R
#define CFG_FT_ENABLE_NAME                              "gFtEnabled"
#define CFG_FT_ENABLE_MIN                               (0)
#define CFG_FT_ENABLE_MAX                               (1)  
#define CFG_FT_ENABLE_DEFAULT                           (0)

#define CFG_FT_RESOURCE_REQ_NAME                        "gFTResourceReqSupported"
#define CFG_FT_RESOURCE_REQ_MIN                         (0)
#define CFG_FT_RESOURCE_REQ_MAX                         (1)
#define CFG_FT_RESOURCE_REQ_DEFAULT                     (0)
#endif

#ifdef WLAN_FEATURE_NEIGHBOR_ROAMING
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_NAME             "gNeighborScanTimerPeriod"
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN              (0)
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX              (1000)
#define CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT          (200)

#define CFG_NEIGHBOR_REASSOC_RSSI_THRESHOLD_NAME              "gNeighborReassocThreshold"
#define CFG_NEIGHBOR_REASSOC_RSSI_THRESHOLD_MIN               (10)
#define CFG_NEIGHBOR_REASSOC_RSSI_THRESHOLD_MAX               (125)
#define CFG_NEIGHBOR_REASSOC_RSSI_THRESHOLD_DEFAULT           (125)

#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_NAME      "gNeighborLookupThreshold"
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN       (10)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX       (120)
#define CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT   (120)

#define CFG_NEIGHBOR_SCAN_CHAN_LIST_NAME                      "gNeighborScanChannelList"
#define CFG_NEIGHBOR_SCAN_CHAN_LIST_DEFAULT                   "1,6"

#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_NAME                  "gNeighborScanChannelMinTime"
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN                   (10)   
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX                   (40)   
#define CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_DEFAULT               (20)   

#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_NAME                  "gNeighborScanChannelMaxTime"
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN                   (10)   
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX                   (40)   
#define CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_DEFAULT               (30)   

#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_NAME           "gMaxNeighborReqTries"
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MIN            (1)
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_MAX            (4)
#define CFG_11R_NEIGHBOR_REQ_MAX_TRIES_DEFAULT        (1)


#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_NAME         "gNeighborScanRefreshPeriod"
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN          (1000)
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX          (60000)
#define CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_DEFAULT      (20000)
#endif /* WLAN_FEATURE_NEIGHBOR_ROAMING */

#define CFG_QOS_WMM_BURST_SIZE_DEFN_NAME                        "burstSizeDefinition" 
#define CFG_QOS_WMM_BURST_SIZE_DEFN_MIN                         (0)
#define CFG_QOS_WMM_BURST_SIZE_DEFN_MAX                         (1)
#define CFG_QOS_WMM_BURST_SIZE_DEFN_DEFAULT                     (0)

#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_NAME                        "tsInfoAckPolicy" 
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_MIN                         (0x00)
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_MAX                         (0x01)
#define CFG_QOS_WMM_TS_INFO_ACK_POLICY_DEFAULT                     (0x00)

#define CFG_SINGLE_TID_RC_NAME                             "SingleTIDRC"
#define CFG_SINGLE_TID_RC_MIN                               (0) // Seperate replay counter for all TID
#define CFG_SINGLE_TID_RC_MAX                               (1) // Single replay counter for all TID 
#define CFG_SINGLE_TID_RC_DEFAULT                           (1) 
#define CFG_MCAST_BCAST_FILTER_SETTING_NAME          "McastBcastFilter"
#define CFG_MCAST_BCAST_FILTER_SETTING_MIN           (0)
#define CFG_MCAST_BCAST_FILTER_SETTING_MAX           (3)
#define CFG_MCAST_BCAST_FILTER_SETTING_DEFAULT       (0)

#define CFG_DYNAMIC_PSPOLL_VALUE_NAME          "gDynamicPSPollvalue"
#define CFG_DYNAMIC_PSPOLL_VALUE_MIN           (0)
#define CFG_DYNAMIC_PSPOLL_VALUE_MAX           (255)
#define CFG_DYNAMIC_PSPOLL_VALUE_DEFAULT       (0)

#define CFG_TELE_BCN_WAKEUP_EN_NAME            "gTelescopicBeaconWakeupEn"
#define CFG_TELE_BCN_WAKEUP_EN_MIN             (0)
#define CFG_TELE_BCN_WAKEUP_EN_MAX             (1)
#define CFG_TELE_BCN_WAKEUP_EN_DEFAULT         (0)

#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_NAME                 "gAddTSWhenACMIsOff"
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MIN                  (0)
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_MAX                  (1) //Send AddTs even when ACM is not set for the AC
#define CFG_QOS_ADDTS_WHEN_ACM_IS_OFF_DEFAULT              (0)


#define CFG_VALIDATE_SCAN_LIST_NAME                 "gValidateScanList"
#define CFG_VALIDATE_SCAN_LIST_MIN                  (0)
#define CFG_VALIDATE_SCAN_LIST_MAX                  (1) 
#define CFG_VALIDATE_SCAN_LIST_DEFAULT              (0)

#define CFG_NULLDATA_AP_RESP_TIMEOUT_NAME       "gNullDataApRespTimeout"
#define CFG_NULLDATA_AP_RESP_TIMEOUT_MIN        ( WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT_STAMIN )
#define CFG_NULLDATA_AP_RESP_TIMEOUT_MAX        ( WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT_STAMAX )
#define CFG_NULLDATA_AP_RESP_TIMEOUT_DEFAULT    ( WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT_STADEF )

#define CFG_AP_DATA_AVAIL_POLL_PERIOD_NAME      "gApDataAvailPollInterval"
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_MIN       ( WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STAMIN )
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_MAX       ( WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STAMAX )
#define CFG_AP_DATA_AVAIL_POLL_PERIOD_DEFAULT   ( WNI_CFG_AP_DATA_AVAIL_POLL_PERIOD_STADEF )

#define CFG_ENABLE_HOST_ARPOFFLOAD_NAME         "hostArpOffload"
#define CFG_ENABLE_HOST_ARPOFFLOAD_MIN            ( 0 )
#define CFG_ENABLE_HOST_ARPOFFLOAD_MAX            ( 1 )
#define CFG_ENABLE_HOST_ARPOFFLOAD_DEFAULT        ( 1 )

#define HDD_MCASTBCASTFILTER_FILTER_NONE                    0x00
#define HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST           0x01
#define HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST           0x02
#define HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST 0x03
/*--------------------------------------------------------------------------- 
  Type declarations
  -------------------------------------------------------------------------*/ 

typedef struct
{
   //Bitmap to track what is explicitly configured
   DECLARE_BITMAP(bExplicitCfg, MAX_CFG_INI_ITEMS);

   //Config parameters
   v_U32_t       RTSThreshold;
   v_U32_t       FragmentationThreshold;
   v_U32_t       nCheckForHangTime;
   v_U32_t       Calibration;
   v_U32_t       CalibrationPeriod;
   v_U8_t        OperatingChannel;
   v_BOOL_t      ShortSlotTimeEnabled;
   v_BOOL_t      Is11dSupportEnabled;
   v_BOOL_t      fEnforce11dChannels;
   v_BOOL_t      fEnforceCountryCodeMatch;
   v_BOOL_t      fEnforceDefaultDomain;
   v_U32_t       Cfg1Id;
   v_U32_t       Cfg2Id;
   v_U32_t       Cfg3Id;
   v_U32_t       Cfg4Id;
   v_U32_t       Cfg5Id;
   v_U32_t       Cfg1Value;
   v_U32_t       Cfg2Value;
   v_U32_t       Cfg3Value;
   v_U32_t       Cfg4Value;
   v_U32_t       Cfg5Value;
   v_U32_t       HeartbeatThresh24;
   char          PowerUsageControl[4];
   v_U8_t        nEnableSuspend;
   v_U8_t        nEnableDriverStop;
   v_BOOL_t      fIsImpsEnabled;
   v_BOOL_t      fIsLogpEnabled;
   v_U8_t        btcExecutionMode;
   v_U8_t        btcConsBtSlotsToBlockDuringDhcp;
   v_U8_t        btcA2DPBtSubIntervalsDuringDhcp;
   v_U32_t       nImpsModSleepTime;
   v_U32_t       nImpsMaxSleepTime;
   v_U32_t       nImpsMinSleepTime;
   v_BOOL_t      fIsBmpsEnabled;
   v_U32_t       nBmpsModListenInterval;
   v_U32_t       nBmpsMaxListenInterval;
   v_U32_t       nBmpsMinListenInterval;
   v_BOOL_t      fIsAutoBmpsTimerEnabled;
   v_U32_t       nAutoBmpsTimerValue;
   eHddDot11Mode dot11Mode;
   v_U32_t       ChannelBondingMode;
   v_U32_t       MaxRxAmpduFactor;
   v_U32_t       nBAAgingTimerInterval;
   v_U16_t       TxRate;
   v_U32_t       AdaptiveThresholdAlgo;
   v_U32_t       ShortGI20MhzEnable;
   v_U32_t       BlockAckAutoSetup;
   v_U32_t       ScanResultAgeCount;
   v_U32_t       nScanAgeTimeNCNPS;
   v_U32_t       nScanAgeTimeNCPS;
   v_U32_t       nScanAgeTimeCNPS;
   v_U32_t       nScanAgeTimeCPS;
   v_U8_t        nRssiCatGap;
   v_U32_t       nStatTimerInterval;
   v_BOOL_t      fIsShortPreamble;
   v_BOOL_t      fIsAutoIbssBssid;
   v_MACADDR_t   IbssBssid;
   
   v_U8_t        intfAddrMask;
   v_MACADDR_t   intfMacAddr[VOS_MAX_CONCURRENCY_PERSONA];

#ifdef WLAN_SOFTAP_FEATURE
   v_BOOL_t      apUapsdEnabled;
   v_BOOL_t      apProtEnabled;
   v_U16_t       apProtection;
   v_BOOL_t      apOBSSProtEnabled;
   v_U8_t        MinFramesProcThres;
   v_U8_t        apCntryCode[4];
   v_BOOL_t      apDisableIntraBssFwd;
   v_U8_t        nEnableListenMode;    
   v_U32_t       nAPAutoShutOff;
#endif

   v_U32_t       nBeaconInterval;
   v_U8_t        nTxPowerCap;   //In dBm
   v_BOOL_t      fIsLowGainOverride;

#if defined WLAN_FEATURE_VOWIFI
   v_BOOL_t      fRrmEnable;
   v_U8_t        nInChanMeasMaxDuration;
   v_U8_t        nOutChanMeasMaxDuration;
   v_U16_t       nRrmRandnIntvl;
#endif

#ifdef WLAN_FEATURE_VOWIFI_11R
   //Vowifi 11r params
   v_BOOL_t      fFTEnable;
   v_BOOL_t      fFTResourceReqSupported;
#endif

#ifdef WLAN_FEATURE_NEIGHBOR_ROAMING
   v_U16_t       nNeighborScanPeriod;
   v_U16_t       nNeighborReassocRssiThreshold;
   v_U16_t       nNeighborLookupRssiThreshold;
   char          neighborScanChanList[100];         
   v_U16_t       nNeighborScanMinChanTime; 
   v_U16_t       nNeighborScanMaxChanTime; 
   v_U16_t       nMaxNeighborReqTries;
   v_U16_t       nNeighborResultsRefreshPeriod; 
#endif

   //Handoff Parameters
   v_BOOL_t      fIsHandoffEnabled;
#ifdef FEATURE_WLAN_GEN6_ROAMING
   //Handoff : No Wifi Params
   v_U32_t       nRssiFilterConstNoWifi;
   v_U32_t       nChannelScanTime;
   v_U32_t       nRssiThreshNeigh;
   v_U32_t       nRssiThreshAssoc;
   v_U32_t       nActiveScanInterval;
   v_U32_t       nActiveScanDuration;
   //Handoff : No Traffic Params
   v_U32_t       nRssiFilterConstNt;
   v_U32_t       nNumCandidateSetNt;
   v_U32_t       nInactThreshNt;
   v_U32_t       nInactPeriodNt;
   v_U32_t       nBestCandidateApRssiDeltaNt;
   v_U32_t       nNeighApBgScanIntervalNt;
   v_U32_t       nNeighApIncrNt;
   v_U32_t       nRssiThreshCandidateNt;
   v_U32_t       nPmkCacheRssiNt;
   v_U32_t       nRssiThresholdCurrentApGoodNt;
   //Handoff : Non Real Time Params
   v_U32_t        nRssiFilterConstNrt;
   v_U32_t        nNumCandtSetEntryNrt;
   v_U32_t        nRssiThresholdCurrentApGoodNrt;
   v_U32_t        nRssiThresholdCurrentApGoodEmptyCandtsetNrt;
   v_U32_t        nRssiThresholdHoFromCurrentApNrt;
   v_U32_t        nRssiThresholdCandtSetNrt;
   v_U32_t        nBgScanIntervalNrt;
   v_U32_t        nBgScanIncrIntervalNrt;
   v_U32_t        nBgScanDelayIntervalNrt;
   v_U32_t        nPerMsmtIntervalNrt;
   v_U32_t        nPerThresholdHoFromCurrentApNrt;
   v_U32_t        nPmkCacheRssiDeltaNrt;
   v_U32_t        nBestCandidateApRssiDeltaNrt;
   //Handoff : Real Time Params
   v_U32_t        nRssiFilterConstRt;
   v_U32_t        nNumCandtSetEntryRt;
   v_U32_t        nRssiThresholdCurrentApGoodRt;
   v_U32_t        nRssiThresholdHoFromCurrentApRt;
   v_U32_t        nRssiThresholdCandtSetRt;
   v_U32_t        nBgScanIntervalRt;
   v_U32_t        nPerMsmtIntervalRt;
   v_U32_t        nPerThresholdHoFromCurrentApRt;
   v_U32_t        nPmkCacheRssiDeltaRt;
   v_U32_t        nBestCandidateApRssiDeltaRt;
#endif

   //Additional Handoff params
   v_BOOL_t       nEnableIdleScan;
   v_U32_t        nRoamingTime;
   v_U16_t        nVccRssiTrigger;
   v_U32_t        nVccUlMacLossThreshold;

   v_U32_t        nPassiveMinChnTime;    //in units of milliseconds
   v_U32_t        nPassiveMaxChnTime;    //in units of milliseconds
   v_U32_t        nActiveMinChnTime;     //in units of milliseconds
   v_U32_t        nActiveMaxChnTime;     //in units of milliseconds
										 
   v_U8_t         nMaxPsPoll;

   v_U8_t         nRssiFilterPeriod;
   v_BOOL_t       fIgnoreDtim;

   v_U8_t         nRxAnt;
   v_U8_t         fEnableFwHeartBeatMonitoring;
   v_U8_t         fEnableFwBeaconFiltering;
   v_U8_t         fEnableFwRssiMonitoring;
   v_U8_t         nDataInactivityTimeout;
   v_U8_t         nthBeaconFilter;
	
										 
   //WMM QoS Configuration
   hdd_wmm_user_mode_t          WmmMode;
   v_BOOL_t                     b80211eIsEnabled;
   v_U8_t                       UapsdMask;    // what ACs to setup U-APSD for at assoc
   v_U8_t                       MaxSpLength;
   v_U32_t                      InfraUapsdVoSrvIntv;
   v_U32_t                      InfraUapsdVoSuspIntv;
   v_U32_t                      InfraUapsdViSrvIntv;
   v_U32_t                      InfraUapsdViSuspIntv;
   v_U32_t                      InfraUapsdBeSrvIntv;
   v_U32_t                      InfraUapsdBeSuspIntv;
   v_U32_t                      InfraUapsdBkSrvIntv;
   v_U32_t                      InfraUapsdBkSuspIntv;
#ifdef FEATURE_WLAN_CCX
   v_U32_t                      InfraInactivityInterval;
   v_BOOL_t                     isCcxIniFeatureEnabled;
#endif
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_CCX)
   v_U8_t                       FTRssiFilterPeriod;
   v_BOOL_t                     isFastTransitionEnabled;
#endif

   hdd_wmm_classification_t     PktClassificationBasis; // DSCP or 802.1Q
   v_BOOL_t                     bImplicitQosEnabled;

   /* default TSPEC parameters for AC_VO */
   sme_QosWmmDirType            InfraDirAcVo;
   v_U16_t                      InfraNomMsduSizeAcVo;
   v_U32_t                      InfraMeanDataRateAcVo;
   v_U32_t                      InfraMinPhyRateAcVo;
   v_U16_t                      InfraSbaAcVo;

   /* default TSPEC parameters for AC_VI */
   sme_QosWmmDirType            InfraDirAcVi;
   v_U16_t                      InfraNomMsduSizeAcVi;
   v_U32_t                      InfraMeanDataRateAcVi;
   v_U32_t                      InfraMinPhyRateAcVi;
   v_U16_t                      InfraSbaAcVi;

   /* default TSPEC parameters for AC_BE */
   sme_QosWmmDirType            InfraDirAcBe;
   v_U16_t                      InfraNomMsduSizeAcBe;
   v_U32_t                      InfraMeanDataRateAcBe;
   v_U32_t                      InfraMinPhyRateAcBe;
   v_U16_t                      InfraSbaAcBe;

   /* default TSPEC parameters for AC_BK */
   sme_QosWmmDirType            InfraDirAcBk;
   v_U16_t                      InfraNomMsduSizeAcBk;
   v_U32_t                      InfraMeanDataRateAcBk;
   v_U32_t                      InfraMinPhyRateAcBk;
   v_U16_t                      InfraSbaAcBk;

   /* TL related configuration */
   v_U8_t                       WfqBkWeight;
   v_U8_t                       WfqBeWeight;
   v_U8_t                       WfqViWeight;
   v_U8_t                       WfqVoWeight;
   v_U32_t                      DelayedTriggerFrmInt;

   /* Wowl pattern */
   char                        wowlPattern[1024];         
   v_BOOL_t                    b19p2MhzPmicClkEnabled;

   /* Control for Replay counetr. value 1 means 
      single replay counter for all TID*/
   v_BOOL_t                    bSingleTidRc;
   v_U8_t                      mcastBcastFilterSetting;
   v_BOOL_t                    fhostArpOffload;
   v_BOOL_t                     burstSizeDefinition;
   v_U8_t                       tsInfoAckPolicy;
   
   /* RF Settling Time Clock */
   v_U32_t                     rfSettlingTimeUs;

   v_U8_t                      dynamicPsPollValue;
   v_BOOL_t                    AddTSWhenACMIsOff;
   v_BOOL_t                    fValidateScanList;

   v_U32_t                     infraStaKeepAlivePeriod;
   v_U8_t                      nNullDataApRespTimeout;

   v_U32_t                     apDataAvailPollPeriodInMs;
   v_BOOL_t                    teleBcnWakeupEn;
} hdd_config_t;
/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 
VOS_STATUS hdd_parse_config_ini(hdd_context_t *pHddCtx);
VOS_STATUS hdd_set_sme_config( hdd_context_t *pHddCtx );
v_BOOL_t hdd_update_config_dat ( hdd_context_t *pHddCtx );
VOS_STATUS hdd_cfg_get_config(hdd_context_t *pHddCtx, char *pBuf, int buflen);
eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode( eHddDot11Mode dot11Mode );

#define FIELD_OFFSET(__type, __field) ((unsigned int)(&((__type *)0)->__field))
#define VAR_OFFSET( _Struct, _Var ) ( (unsigned int) FIELD_OFFSET(_Struct, _Var ) )
#define VAR_SIZE( _Struct, _Var ) sizeof( ((_Struct *)0)->_Var )

#define VAR_FLAGS_NONE         (      0 )
#define VAR_FLAGS_REQUIRED     ( 1 << 0 )   // bit 0 is Required or Optional
#define VAR_FLAGS_OPTIONAL     ( 0 << 0 )

#define VAR_FLAGS_RANGE_CHECK  ( 1 << 1 )   // bit 1 tells if range checking is required.
                                            // If less than MIN, assume MIN.
                                            // If greater than MAX, assume MAX.

#define VAR_FLAGS_RANGE_CHECK_ASSUME_MINMAX ( VAR_FLAGS_RANGE_CHECK )

#define VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT ( 1 << 2 )  // bit 2 is range checking that assumes the DEFAULT value
                                                         // If less than MIN, assume DEFAULT,
                                                         // If grateer than MAX, assume DEFAULT.

#define VAR_FLAGS_XLATION_REQD ( 1 << 3 ) // Bit 3 indicates that
                                          // translation from a raw
                                          // integer to an enumeration
                                          // is required

#define CFG_BAD_ENUM_XLATION ( 0xffffffff )

typedef enum 
{
  WLAN_PARAM_Integer,
  WLAN_PARAM_HexInteger,
  WLAN_PARAM_String,
  WLAN_PARAM_MacAddr,
}WLAN_PARAMETER_TYPE;

#define REG_VARIABLE( _Name, _Type,  _Struct, _VarName,          \
                      _Flags, _Default, _Min, _Max )             \
{                                                                \
  ( _Name ),                                                     \
  ( _Type ),                                                     \
  ( _Flags ),                                                    \
  VAR_OFFSET( _Struct, _VarName ),                               \
  VAR_SIZE( _Struct, _VarName ),                                 \
  ( _Default ),                                                  \
  ( _Min ),                                                      \
  ( _Max ),                                                      \
  NULL                                                           \
}

#define REG_VARIABLE_STRING( _Name, _Type,  _Struct, _VarName,   \
                             _Flags, _Default )                  \
{                                                                \
  ( _Name ),                                                     \
  ( _Type ),                                                     \
  ( _Flags ),                                                    \
  VAR_OFFSET( _Struct, _VarName ),                               \
  VAR_SIZE( _Struct, _VarName ),                                 \
  (unsigned long)( _Default ),                                   \
  0,                                                             \
  0,                                                             \
  NULL                                                           \
}

typedef struct tREG_TABLE_ENTRY {

  char*               RegName;            // variable name in the qcom_cfg.ini file
  WLAN_PARAMETER_TYPE RegType;            // variable type in the hdd_config_t structure
  unsigned long       Flags;              // Specify optional parms and if RangeCheck is performed
  unsigned short      VarOffset;          // offset to field from the base address of the structure
  unsigned short      VarSize;            // size (in bytes) of the field
  unsigned long       VarDefault;         // default value to use
  unsigned long       VarMin;             // minimum value, for range checking
  unsigned long       VarMax;             // maximum value, for range checking
  unsigned int        (*pfnXlate)(char*); // Reg value in, enumeration out
} REG_TABLE_ENTRY;

static __inline unsigned long utilMin( unsigned long a, unsigned long b )
{
  return( ( a < b ) ? a : b );
}

#endif
