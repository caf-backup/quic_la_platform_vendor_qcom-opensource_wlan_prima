
/**========================================================================= 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$   $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  07/27/09    kanand Created module. 

  ==========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/


#include <linux/firmware.h>
#include <linux/string.h>
#include <wlan_hdd_includes.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_assoc.h>
#include <linux/mmc/sdio_func.h>
#include <wlan_hdd_cfg.h>
#include <linux/string.h>
#include <vos_types.h>
#include <csrApi.h>
#include <pmcApi.h>

//Number of items that can be overirden in qcom_cfg.ini file
#define MAX_CFG_INI_ITEMS   128

#define MASK_BITS_OFF( _Field, _Bitmask ) ( (_Field) &= ~(_Bitmask) )


REG_TABLE_ENTRY g_registry_table[] =
{
   REG_VARIABLE( CFG_RTS_THRESHOLD_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, RTSThreshold, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RTS_THRESHOLD_DEFAULT, 
                 CFG_RTS_THRESHOLD_MIN, 
                 CFG_RTS_THRESHOLD_MAX ), 

   REG_VARIABLE( CFG_FRAG_THRESHOLD_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, FragmentationThreshold, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_FRAG_THRESHOLD_DEFAULT, 
                 CFG_FRAG_THRESHOLD_MIN, 
                 CFG_FRAG_THRESHOLD_MAX ),              

   REG_VARIABLE( CFG_CALIBRATION_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Calibration, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_CALIBRATION_DEFAULT, 
                 CFG_CALIBRATION_MIN, 
                 CFG_CALIBRATION_MAX ),
                
   REG_VARIABLE( CFG_CALIBRATION_PERIOD_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, CalibrationPeriod, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_CALIBRATION_PERIOD_DEFAULT, 
                 CFG_CALIBRATION_PERIOD_MIN, 
                 CFG_CALIBRATION_PERIOD_MAX ), 

   REG_VARIABLE( CFG_OPERATING_CHANNEL_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, OperatingChannel,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_OPERATING_CHANNEL_DEFAULT, 
                 CFG_OPERATING_CHANNEL_MIN, 
                 CFG_OPERATING_CHANNEL_MAX ),

   REG_VARIABLE( CFG_SHORT_SLOT_TIME_ENABLED_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, ShortSlotTimeEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_SHORT_SLOT_TIME_ENABLED_DEFAULT, 
                 CFG_SHORT_SLOT_TIME_ENABLED_MIN, 
                 CFG_SHORT_SLOT_TIME_ENABLED_MAX ),

   REG_VARIABLE( CFG_11D_SUPPORT_ENABLED_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Is11dSupportEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_11D_SUPPORT_ENABLED_DEFAULT, 
                 CFG_11D_SUPPORT_ENABLED_MIN, 
                 CFG_11D_SUPPORT_ENABLED_MAX ),

   REG_VARIABLE( CFG_ENFORCE_11D_CHANNELS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fEnforce11dChannels, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_ENFORCE_11D_CHANNELS_DEFAULT, 
                 CFG_ENFORCE_11D_CHANNELS_MIN, 
                 CFG_ENFORCE_11D_CHANNELS_MAX ),

   REG_VARIABLE( CFG_ENFORCE_COUNTRY_CODE_MATCH_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fEnforceCountryCodeMatch, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_ENFORCE_COUNTRY_CODE_MATCH_DEFAULT, 
                 CFG_ENFORCE_COUNTRY_CODE_MATCH_MIN, 
                 CFG_ENFORCE_COUNTRY_CODE_MATCH_MAX ),

   REG_VARIABLE( CFG_ENFORCE_DEFAULT_DOMAIN_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fEnforceDefaultDomain, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_ENFORCE_DEFAULT_DOMAIN_DEFAULT, 
                 CFG_ENFORCE_DEFAULT_DOMAIN_MIN, 
                 CFG_ENFORCE_DEFAULT_DOMAIN_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_ID1_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg1Id,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_ID1_DEFAULT, 
                 CFG_GENERIC_ID1_MIN, 
                 CFG_GENERIC_ID1_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_ID2_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg2Id,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_ID2_DEFAULT, 
                 CFG_GENERIC_ID2_MIN, 
                 CFG_GENERIC_ID2_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_ID3_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg3Id,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_ID3_DEFAULT, 
                 CFG_GENERIC_ID3_MIN, 
                 CFG_GENERIC_ID3_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_ID4_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg4Id,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_ID4_DEFAULT, 
                 CFG_GENERIC_ID4_MIN, 
                 CFG_GENERIC_ID4_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_ID5_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg5Id,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_ID5_DEFAULT, 
                 CFG_GENERIC_ID5_MIN, 
                 CFG_GENERIC_ID5_MAX ),

   REG_VARIABLE( CFG_GENERIC_VALUE1_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg1Value,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_VALUE1_DEFAULT, 
                 CFG_GENERIC_VALUE1_MIN, 
                 CFG_GENERIC_VALUE1_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_VALUE2_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg2Value,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_VALUE2_DEFAULT, 
                 CFG_GENERIC_VALUE2_MIN, 
                 CFG_GENERIC_VALUE2_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_VALUE3_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg3Value,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_VALUE3_DEFAULT, 
                 CFG_GENERIC_VALUE3_MIN, 
                 CFG_GENERIC_VALUE3_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_VALUE4_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg4Value,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_VALUE4_DEFAULT, 
                 CFG_GENERIC_VALUE4_MIN, 
                 CFG_GENERIC_VALUE4_MAX ),
                
   REG_VARIABLE( CFG_GENERIC_VALUE5_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, Cfg5Value,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_GENERIC_VALUE5_DEFAULT, 
                 CFG_GENERIC_VALUE5_MIN, 
                 CFG_GENERIC_VALUE5_MAX ),

   REG_VARIABLE( CFG_HEARTBEAT_THRESH_24_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, HeartbeatThresh24, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_HEARTBEAT_THRESH_24_DEFAULT, 
                 CFG_HEARTBEAT_THRESH_24_MIN, 
                 CFG_HEARTBEAT_THRESH_24_MAX ),
                
   REG_VARIABLE_STRING( CFG_POWER_USAGE_NAME, WLAN_PARAM_String,
                        hdd_config_t, PowerUsageControl, 
                        VAR_FLAGS_OPTIONAL,
                        (void *)CFG_POWER_USAGE_DEFAULT ),

   REG_VARIABLE( CFG_ENABLE_SUSPEND_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nEnableSuspend, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ENABLE_SUSPEND_DEFAULT, 
                 CFG_ENABLE_SUSPEND_MIN, 
                 CFG_ENABLE_SUSPEND_MAX ),

   REG_VARIABLE( CFG_ENABLE_IMPS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsImpsEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ENABLE_IMPS_DEFAULT, 
                 CFG_ENABLE_IMPS_MIN, 
                 CFG_ENABLE_IMPS_MAX ),

   REG_VARIABLE( CFG_IMPS_MINIMUM_SLEEP_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nImpsMinSleepTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_IMPS_MINIMUM_SLEEP_TIME_DEFAULT, 
                 CFG_IMPS_MINIMUM_SLEEP_TIME_MIN, 
                 CFG_IMPS_MINIMUM_SLEEP_TIME_MAX ),

   REG_VARIABLE( CFG_IMPS_MAXIMUM_SLEEP_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nImpsMaxSleepTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_IMPS_MAXIMUM_SLEEP_TIME_DEFAULT, 
                 CFG_IMPS_MAXIMUM_SLEEP_TIME_MIN, 
                 CFG_IMPS_MAXIMUM_SLEEP_TIME_MAX ),

   REG_VARIABLE( CFG_IMPS_MODERATE_SLEEP_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nImpsModSleepTime,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_IMPS_MODERATE_SLEEP_TIME_DEFAULT, 
                 CFG_IMPS_MODERATE_SLEEP_TIME_MIN, 
                 CFG_IMPS_MODERATE_SLEEP_TIME_MAX ),

   REG_VARIABLE( CFG_ENABLE_BMPS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsBmpsEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ENABLE_BMPS_DEFAULT, 
                 CFG_ENABLE_BMPS_MIN, 
                 CFG_ENABLE_BMPS_MAX ),

   REG_VARIABLE( CFG_BMPS_MINIMUM_LI_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nBmpsMinListenInterval, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BMPS_MINIMUM_LI_DEFAULT, 
                 CFG_BMPS_MINIMUM_LI_MIN, 
                 CFG_BMPS_MINIMUM_LI_MAX ),

   REG_VARIABLE( CFG_BMPS_MAXIMUM_LI_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nBmpsMaxListenInterval, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BMPS_MAXIMUM_LI_DEFAULT, 
                 CFG_BMPS_MAXIMUM_LI_MIN, 
                 CFG_BMPS_MAXIMUM_LI_MAX ),

   REG_VARIABLE( CFG_BMPS_MODERATE_LI_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nBmpsModListenInterval,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BMPS_MODERATE_LI_DEFAULT, 
                 CFG_BMPS_MODERATE_LI_MIN, 
                 CFG_BMPS_MODERATE_LI_MAX ),

   REG_VARIABLE( CFG_ENABLE_AUTO_BMPS_TIMER_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsAutoBmpsTimerEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ENABLE_AUTO_BMPS_TIMER_DEFAULT, 
                 CFG_ENABLE_AUTO_BMPS_TIMER_MIN, 
                 CFG_ENABLE_AUTO_BMPS_TIMER_MAX ),

   REG_VARIABLE( CFG_AUTO_BMPS_TIMER_VALUE_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nAutoBmpsTimerValue, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_AUTO_BMPS_TIMER_VALUE_DEFAULT, 
                 CFG_AUTO_BMPS_TIMER_VALUE_MIN, 
                 CFG_AUTO_BMPS_TIMER_VALUE_MAX ),

   REG_VARIABLE( CFG_DOT11_MODE_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, dot11Mode, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_DOT11_MODE_DEFAULT, 
                 CFG_DOT11_MODE_MIN, 
                 CFG_DOT11_MODE_MAX ),
                 
   REG_VARIABLE( CFG_CHANNEL_BONDING_MODE_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, ChannelBondingMode, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_CHANNEL_BONDING_MODE_DEFAULT, 
                 CFG_CHANNEL_BONDING_MODE_MIN, 
                 CFG_CHANNEL_BONDING_MODE_MAX),
              
   REG_VARIABLE( CFG_MAX_RX_AMPDU_FACTOR_NAME, WLAN_PARAM_Integer,   
                 hdd_config_t, MaxRxAmpduFactor, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK , 
                 CFG_MAX_RX_AMPDU_FACTOR_DEFAULT, 
                 CFG_MAX_RX_AMPDU_FACTOR_MIN, 
                 CFG_MAX_RX_AMPDU_FACTOR_MAX),
                
   REG_VARIABLE( CFG_FIXED_RATE_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, TxRate, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_FIXED_RATE_DEFAULT, 
                 CFG_FIXED_RATE_MIN, 
                 CFG_FIXED_RATE_MAX ),

   REG_VARIABLE( CFG_SHORT_GI_20MHZ_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, ShortGI20MhzEnable, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_SHORT_GI_20MHZ_DEFAULT, 
                 CFG_SHORT_GI_20MHZ_MIN, 
                 CFG_SHORT_GI_20MHZ_MAX ),

   REG_VARIABLE( CFG_BLOCK_ACK_AUTO_SETUP_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, BlockAckAutoSetup, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_BLOCK_ACK_AUTO_SETUP_DEFAULT, 
                 CFG_BLOCK_ACK_AUTO_SETUP_MIN, 
                 CFG_BLOCK_ACK_AUTO_SETUP_MAX ),
  
   REG_VARIABLE( CFG_SCAN_RESULT_AGE_COUNT_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, ScanResultAgeCount, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_SCAN_RESULT_AGE_COUNT_DEFAULT, 
                 CFG_SCAN_RESULT_AGE_COUNT_MIN, 
                 CFG_SCAN_RESULT_AGE_COUNT_MAX ),

   REG_VARIABLE( CFG_SCAN_RESULT_AGE_TIME_NCNPS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nScanAgeTimeNCNPS, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_SCAN_RESULT_AGE_TIME_NCNPS_DEFAULT, 
                 CFG_SCAN_RESULT_AGE_TIME_NCNPS_MIN, 
                 CFG_SCAN_RESULT_AGE_TIME_NCNPS_MAX ),

   REG_VARIABLE( CFG_SCAN_RESULT_AGE_TIME_NCPS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nScanAgeTimeNCPS, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_SCAN_RESULT_AGE_TIME_NCPS_DEFAULT, 
                 CFG_SCAN_RESULT_AGE_TIME_NCPS_MIN, 
                 CFG_SCAN_RESULT_AGE_TIME_NCPS_MAX ),

   REG_VARIABLE( CFG_SCAN_RESULT_AGE_TIME_CNPS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nScanAgeTimeCNPS, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_SCAN_RESULT_AGE_TIME_CNPS_DEFAULT, 
                 CFG_SCAN_RESULT_AGE_TIME_CNPS_MIN, 
                 CFG_SCAN_RESULT_AGE_TIME_CNPS_MAX ),

   REG_VARIABLE( CFG_SCAN_RESULT_AGE_TIME_CPS_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nScanAgeTimeCPS, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_SCAN_RESULT_AGE_TIME_CPS_DEFAULT, 
                 CFG_SCAN_RESULT_AGE_TIME_CPS_MIN, 
                 CFG_SCAN_RESULT_AGE_TIME_CPS_MAX ),

   REG_VARIABLE( CFG_RSSI_CATEGORY_GAP_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nRssiCatGap, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_CATEGORY_GAP_DEFAULT, 
                 CFG_RSSI_CATEGORY_GAP_MIN, 
                 CFG_RSSI_CATEGORY_GAP_MAX ),  

   REG_VARIABLE( CFG_STAT_TIMER_INTERVAL_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nStatTimerInterval, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_STAT_TIMER_INTERVAL_DEFAULT, 
                 CFG_STAT_TIMER_INTERVAL_MIN, 
                 CFG_STAT_TIMER_INTERVAL_MAX ),

   REG_VARIABLE( CFG_SHORT_PREAMBLE_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsShortPreamble, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_SHORT_PREAMBLE_DEFAULT, 
                 CFG_SHORT_PREAMBLE_MIN, 
                 CFG_SHORT_PREAMBLE_MAX ),

   REG_VARIABLE( CFG_IBSS_AUTO_BSSID_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsAutoIbssBssid, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_IBSS_AUTO_BSSID_DEFAULT, 
                 CFG_IBSS_AUTO_BSSID_MIN, 
                 CFG_IBSS_AUTO_BSSID_MAX ),

   REG_VARIABLE_STRING( CFG_IBSS_BSSID_NAME, WLAN_PARAM_MacAddr,
                        hdd_config_t, IbssBssid, 
                        VAR_FLAGS_OPTIONAL,
                        (void *)CFG_IBSS_BSSID_DEFAULT ),

   REG_VARIABLE_STRING( CFG_STA_MAC_ADDR_NAME, WLAN_PARAM_MacAddr,
                        hdd_config_t, staMacAddr, 
                        VAR_FLAGS_OPTIONAL,
                        (void *)CFG_STA_MAC_ADDR_DEFAULT ),

   REG_VARIABLE( CFG_BEACON_INTERVAL_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nBeaconInterval, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK, 
                 CFG_BEACON_INTERVAL_DEFAULT, 
                 CFG_BEACON_INTERVAL_MIN, 
                 CFG_BEACON_INTERVAL_MAX ),

   REG_VARIABLE( CFG_ENABLE_HANDOFF_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsHandoffEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ENABLE_HANDOFF_DEFAULT, 
                 CFG_ENABLE_HANDOFF_MIN, 
                 CFG_ENABLE_HANDOFF_MAX ),

#ifdef FEATURE_WLAN_GEN6_ROAMING
   REG_VARIABLE( CFG_RSSI_FILTER_CONST_NO_WIFI_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiFilterConstNoWifi, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_NO_WIFI_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_NO_WIFI_MIN, 
                 CFG_RSSI_FILTER_CONST_NO_WIFI_MAX ),

   REG_VARIABLE( CFG_CHANNEL_SCAN_TIME_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nChannelScanTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_CHANNEL_SCAN_TIME_DEFAULT, 
                 CFG_CHANNEL_SCAN_TIME_MIN, 
                 CFG_CHANNEL_SCAN_TIME_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_NEIGH_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThreshNeigh, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_NEIGH_DEFAULT, 
                 CFG_RSSI_THRESH_NEIGH_MIN, 
                 CFG_RSSI_THRESH_NEIGH_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_ASSOC_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThreshAssoc, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_ASSOC_DEFAULT, 
                 CFG_RSSI_THRESH_ASSOC_MIN, 
                 CFG_RSSI_THRESH_ASSOC_MAX ),

   REG_VARIABLE( CFG_ACTIVE_SCAN_INTERVAL_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nActiveScanInterval, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ACTIVE_SCAN_INTERVAL_DEFAULT, 
                 CFG_ACTIVE_SCAN_INTERVAL_MIN, 
                 CFG_ACTIVE_SCAN_INTERVAL_MAX ),

   REG_VARIABLE( CFG_ACTIVE_SCAN_DURATION_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nActiveScanDuration, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ACTIVE_SCAN_DURATION_DEFAULT, 
                 CFG_ACTIVE_SCAN_DURATION_MIN, 
                 CFG_ACTIVE_SCAN_DURATION_MAX ),

   REG_VARIABLE( CFG_RSSI_FILTER_CONST_NT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiFilterConstNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_NT_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_NT_MIN, 
                 CFG_RSSI_FILTER_CONST_NT_MAX ),

   REG_VARIABLE( CFG_NUM_CANDIDATE_SET_NT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nNumCandidateSetNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_NUM_CANDIDATE_SET_NT_DEFAULT, 
                 CFG_NUM_CANDIDATE_SET_NT_MIN, 
                 CFG_NUM_CANDIDATE_SET_NT_MAX ),

   REG_VARIABLE( CFG_INACT_THRESH_NT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nInactThreshNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_INACT_THRESH_NT_DEFAULT, 
                 CFG_INACT_THRESH_NT_MIN, 
                 CFG_INACT_THRESH_NT_MAX ),

   REG_VARIABLE( CFG_INACT_PERIOD_NT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nInactPeriodNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_INACT_PERIOD_NT_DEFAULT, 
                 CFG_INACT_PERIOD_NT_MIN, 
                 CFG_INACT_PERIOD_NT_MAX ),

   REG_VARIABLE( CFG_BEST_CANDT_AP_RSSI_DELTA_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nBestCandidateApRssiDeltaNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BEST_CANDT_AP_RSSI_DELTA_DEFAULT, 
                 CFG_BEST_CANDT_AP_RSSI_DELTA_MIN, 
                 CFG_BEST_CANDT_AP_RSSI_DELTA_MAX ),
   
   REG_VARIABLE( CFG_NEIGH_AP_BG_SCAN_INTERVAL_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nNeighApBgScanIntervalNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_NEIGH_AP_BG_SCAN_INTERVAL_DEFAULT, 
                 CFG_NEIGH_AP_BG_SCAN_INTERVAL_MIN, 
                 CFG_NEIGH_AP_BG_SCAN_INTERVAL_MAX ),

   REG_VARIABLE( CFG_NEIGH_AP_INCR_NT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nNeighApIncrNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_NEIGH_AP_INCR_NT_DEFAULT, 
                 CFG_NEIGH_AP_INCR_NT_MIN, 
                 CFG_NEIGH_AP_INCR_NT_MAX ),
   
   REG_VARIABLE( CFG_RSSI_THRESH_CANDT_NT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nRssiThreshCandidateNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_CANDT_NT_DEFAULT, 
                 CFG_RSSI_THRESH_CANDT_NT_MIN, 
                 CFG_RSSI_THRESH_CANDT_NT_MAX ),

   REG_VARIABLE( CFG_PMK_THRESH_RSSI_NT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nPmkCacheRssiNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PMK_THRESH_RSSI_NT_DEFAULT, 
                 CFG_PMK_THRESH_RSSI_NT_MIN, 
                 CFG_PMK_THRESH_RSSI_NT_MAX ),

  REG_VARIABLE( CFG_RSSI_THRESH_CURR_AP_NT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdCurrentApGoodNt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_CURR_AP_NT_DEFAULT, 
                 CFG_RSSI_THRESH_CURR_AP_NT_MIN, 
                 CFG_RSSI_THRESH_CURR_AP_NT_MAX ),
   
   REG_VARIABLE( CFG_RSSI_FILTER_CONST_NRT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nRssiFilterConstNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_NRT_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_NRT_MIN, 
                 CFG_RSSI_FILTER_CONST_NRT_MAX ),

   REG_VARIABLE( CFG_NUM_CANDIDATE_SET_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nNumCandtSetEntryNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_NUM_CANDIDATE_SET_NRT_DEFAULT, 
                 CFG_NUM_CANDIDATE_SET_NRT_MIN, 
                 CFG_NUM_CANDIDATE_SET_NRT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_CURR_AP_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdCurrentApGoodNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_CURR_AP_NRT_DEFAULT, 
                 CFG_RSSI_THRESH_CURR_AP_NRT_MIN, 
                 CFG_RSSI_THRESH_CURR_AP_NRT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_EMPTY_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdCurrentApGoodEmptyCandtsetNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_EMPTY_NRT_DEFAULT, 
                 CFG_RSSI_THRESH_EMPTY_NRT_MIN, 
                 CFG_RSSI_THRESH_EMPTY_NRT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_HO_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdHoFromCurrentApNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_HO_NRT_DEFAULT, 
                 CFG_RSSI_THRESH_HO_NRT_MIN, 
                 CFG_RSSI_THRESH_HO_NRT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_CDT_SET_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdCandtSetNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_CDT_SET_NRT_DEFAULT, 
                 CFG_RSSI_THRESH_CDT_SET_NRT_MIN, 
                 CFG_RSSI_THRESH_CDT_SET_NRT_MAX ),

   REG_VARIABLE( CFG_BG_SCAN_INTERVAL_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nBgScanIntervalNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BG_SCAN_INTERVAL_NRT_DEFAULT, 
                 CFG_BG_SCAN_INTERVAL_NRT_MIN, 
                 CFG_BG_SCAN_INTERVAL_NRT_MAX ),

   REG_VARIABLE( CFG_BG_SCAN_INCR_INTERVAL_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nBgScanIncrIntervalNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BG_SCAN_INCR_INTERVAL_NRT_DEFAULT, 
                 CFG_BG_SCAN_INCR_INTERVAL_NRT_MIN, 
                 CFG_BG_SCAN_INCR_INTERVAL_NRT_MAX ),

   REG_VARIABLE( CFG_BG_SCAN_DELAY_INTERVAL_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nBgScanDelayIntervalNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BG_SCAN_DELAY_INTERVAL_NRT_DEFAULT, 
                 CFG_BG_SCAN_DELAY_INTERVAL_NRT_MIN, 
                 CFG_BG_SCAN_DELAY_INTERVAL_NRT_MAX ),

   REG_VARIABLE( CFG_PER_MSMT_INTERVAL_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nPerMsmtIntervalNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PER_MSMT_INTERVAL_NRT_DEFAULT, 
                 CFG_PER_MSMT_INTERVAL_NRT_MIN, 
                 CFG_PER_MSMT_INTERVAL_NRT_MAX ),

   REG_VARIABLE( CFG_HO_FROM_CURR_AP_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nPerThresholdHoFromCurrentApNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_HO_FROM_CURR_AP_NRT_DEFAULT, 
                 CFG_HO_FROM_CURR_AP_NRT_MIN, 
                 CFG_HO_FROM_CURR_AP_NRT_MAX ),

   REG_VARIABLE( CFG_PMK_CACHE_RSSI_DELTA_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nPmkCacheRssiDeltaNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PMK_CACHE_RSSI_DELTA_NRT_DEFAULT, 
                 CFG_PMK_CACHE_RSSI_DELTA_NRT_MIN, 
                 CFG_PMK_CACHE_RSSI_DELTA_NRT_MAX ),

   REG_VARIABLE( CFG_BEST_CDT_AP_RSSI_DELTA_NRT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nBestCandidateApRssiDeltaNrt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BEST_CDT_AP_RSSI_DELTA_NRT_DEFAULT, 
                 CFG_BEST_CDT_AP_RSSI_DELTA_NRT_MIN, 
                 CFG_BEST_CDT_AP_RSSI_DELTA_NRT_MAX ),

   REG_VARIABLE( CFG_RSSI_FILTER_CONST_RT_NAME , WLAN_PARAM_Integer, 
                 hdd_config_t, nRssiFilterConstRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_RT_DEFAULT, 
                 CFG_RSSI_FILTER_CONST_RT_MIN, 
                 CFG_RSSI_FILTER_CONST_RT_MAX ),

   REG_VARIABLE( CFG_NUM_CANDIDATE_SET_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nNumCandtSetEntryRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_NUM_CANDIDATE_SET_RT_DEFAULT, 
                 CFG_NUM_CANDIDATE_SET_RT_MIN, 
                 CFG_NUM_CANDIDATE_SET_RT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_CURR_AP_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdCurrentApGoodRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_CURR_AP_RT_DEFAULT, 
                 CFG_RSSI_THRESH_CURR_AP_RT_MIN, 
                 CFG_RSSI_THRESH_CURR_AP_RT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_HO_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdHoFromCurrentApRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_HO_RT_DEFAULT, 
                 CFG_RSSI_THRESH_HO_RT_MIN, 
                 CFG_RSSI_THRESH_HO_RT_MAX ),

   REG_VARIABLE( CFG_RSSI_THRESH_CDT_SET_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRssiThresholdCandtSetRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_THRESH_CDT_SET_RT_DEFAULT, 
                 CFG_RSSI_THRESH_CDT_SET_RT_MIN, 
                 CFG_RSSI_THRESH_CDT_SET_RT_MAX ),

   REG_VARIABLE( CFG_BG_SCAN_INTERVAL_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nBgScanIntervalRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BG_SCAN_INTERVAL_RT_DEFAULT, 
                 CFG_BG_SCAN_INTERVAL_RT_MIN, 
                 CFG_BG_SCAN_INTERVAL_RT_MAX ),

   REG_VARIABLE( CFG_PER_MSMT_INTERVAL_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nPerMsmtIntervalRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PER_MSMT_INTERVAL_RT_DEFAULT, 
                 CFG_PER_MSMT_INTERVAL_RT_MIN, 
                 CFG_PER_MSMT_INTERVAL_RT_MAX ),

   REG_VARIABLE( CFG_HO_FROM_CURR_AP_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nPerThresholdHoFromCurrentApRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_HO_FROM_CURR_AP_RT_DEFAULT, 
                 CFG_HO_FROM_CURR_AP_RT_MIN, 
                 CFG_HO_FROM_CURR_AP_RT_MAX ),

   REG_VARIABLE( CFG_PMK_CACHE_RSSI_DELTA_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nPmkCacheRssiDeltaRt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PMK_CACHE_RSSI_DELTA_RT_DEFAULT, 
                 CFG_PMK_CACHE_RSSI_DELTA_RT_MIN, 
                 CFG_PMK_CACHE_RSSI_DELTA_RT_MAX ),

   REG_VARIABLE( CFG_BEST_CDT_AP_RSSI_DELTA_RT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nBestCandidateApRssiDeltaRt,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_BEST_CDT_AP_RSSI_DELTA_RT_DEFAULT, 
                 CFG_BEST_CDT_AP_RSSI_DELTA_RT_MIN, 
                 CFG_BEST_CDT_AP_RSSI_DELTA_RT_MAX ),

#endif //#ifdef FEATURE_WLAN_GEN6_ROAMING

   REG_VARIABLE( CFG_ENABLE_IDLE_SCAN_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nEnableIdleScan, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ENABLE_IDLE_SCAN_DEFAULT, 
                 CFG_ENABLE_IDLE_SCAN_MIN, 
                 CFG_ENABLE_IDLE_SCAN_MAX ),

   REG_VARIABLE( CFG_ROAMING_TIME_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nRoamingTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ROAMING_TIME_DEFAULT, 
                 CFG_ROAMING_TIME_MIN, 
                 CFG_ROAMING_TIME_MAX ),

   REG_VARIABLE( CFG_VCC_RSSI_TRIGGER_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nVccRssiTrigger, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_VCC_RSSI_TRIGGER_DEFAULT, 
                 CFG_VCC_RSSI_TRIGGER_MIN, 
                 CFG_VCC_RSSI_TRIGGER_MAX ),

   REG_VARIABLE( CFG_VCC_UL_MAC_LOSS_THRESH_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, nVccUlMacLossThreshold, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_VCC_UL_MAC_LOSS_THRESH_DEFAULT, 
                 CFG_VCC_UL_MAC_LOSS_THRESH_MIN, 
                 CFG_VCC_UL_MAC_LOSS_THRESH_MAX ),

   REG_VARIABLE( CFG_PASSIVE_MAX_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nPassiveMaxChnTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PASSIVE_MAX_CHANNEL_TIME_DEFAULT, 
                 CFG_PASSIVE_MAX_CHANNEL_TIME_MIN, 
                 CFG_PASSIVE_MAX_CHANNEL_TIME_MAX ),

   REG_VARIABLE( CFG_PASSIVE_MIN_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nPassiveMinChnTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_PASSIVE_MIN_CHANNEL_TIME_DEFAULT, 
                 CFG_PASSIVE_MIN_CHANNEL_TIME_MIN, 
                 CFG_PASSIVE_MIN_CHANNEL_TIME_MAX ),

   REG_VARIABLE( CFG_ACTIVE_MAX_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nActiveMaxChnTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ACTIVE_MAX_CHANNEL_TIME_DEFAULT, 
                 CFG_ACTIVE_MAX_CHANNEL_TIME_MIN, 
                 CFG_ACTIVE_MAX_CHANNEL_TIME_MAX ),

   REG_VARIABLE( CFG_ACTIVE_MIN_CHANNEL_TIME_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nActiveMinChnTime, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_ACTIVE_MIN_CHANNEL_TIME_DEFAULT, 
                 CFG_ACTIVE_MIN_CHANNEL_TIME_MIN, 
                 CFG_ACTIVE_MIN_CHANNEL_TIME_MAX ),
   
   REG_VARIABLE( CFG_MAX_PS_POLL_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nMaxPsPoll, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_MAX_PS_POLL_DEFAULT, 
                 CFG_MAX_PS_POLL_MIN, 
                 CFG_MAX_PS_POLL_MAX ),

    REG_VARIABLE( CFG_MAX_TX_POWER_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nTxPowerCap, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_MAX_TX_POWER_DEFAULT, 
                 CFG_MAX_TX_POWER_MIN, 
                 CFG_MAX_TX_POWER_MAX ),

   REG_VARIABLE( CFG_LOW_GAIN_OVERRIDE_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIsLowGainOverride,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_LOW_GAIN_OVERRIDE_DEFAULT, 
                 CFG_LOW_GAIN_OVERRIDE_MIN, 
                 CFG_LOW_GAIN_OVERRIDE_MAX ),

   REG_VARIABLE( CFG_RSSI_FILTER_PERIOD_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, nRssiFilterPeriod,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_RSSI_FILTER_PERIOD_DEFAULT, 
                 CFG_RSSI_FILTER_PERIOD_MIN, 
                 CFG_RSSI_FILTER_PERIOD_MAX ),

   REG_VARIABLE( CFG_IGNORE_DTIM_NAME, WLAN_PARAM_Integer,
                 hdd_config_t, fIgnoreDtim,
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_IGNORE_DTIM_DEFAULT, 
                 CFG_IGNORE_DTIM_MIN, 
                 CFG_IGNORE_DTIM_MAX ),
    
   REG_VARIABLE( CFG_QOS_WMM_MODE_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, WmmMode, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_MODE_DEFAULT, 
                 CFG_QOS_WMM_MODE_MIN, 
                 CFG_QOS_WMM_MODE_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_80211E_ENABLED_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, b80211eIsEnabled, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_80211E_ENABLED_DEFAULT, 
                 CFG_QOS_WMM_80211E_ENABLED_MIN, 
                 CFG_QOS_WMM_80211E_ENABLED_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_UAPSD_MASK_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, UapsdMask, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_UAPSD_MASK_DEFAULT, 
                 CFG_QOS_WMM_UAPSD_MASK_MIN, 
                 CFG_QOS_WMM_UAPSD_MASK_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_MAX_SP_LEN_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, MaxSpLength, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_MAX_SP_LEN_DEFAULT, 
                 CFG_QOS_WMM_MAX_SP_LEN_MIN, 
                 CFG_QOS_WMM_MAX_SP_LEN_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdVoSrvIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_VO_SRV_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdVoSuspIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_VO_SUS_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdViSrvIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_VI_SRV_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdViSuspIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_VI_SUS_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdBeSrvIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_BE_SRV_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdBeSuspIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_BE_SUS_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdBkSrvIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_BK_SRV_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraUapsdBkSuspIntv, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_DEFAULT, 
                 CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MIN, 
                 CFG_QOS_WMM_INFRA_UAPSD_BK_SUS_INTV_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_PKT_CLASSIFY_BASIS_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, PktClassificationBasis, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_PKT_CLASSIFY_BASIS_DEFAULT, 
                 CFG_QOS_WMM_PKT_CLASSIFY_BASIS_MIN, 
                 CFG_QOS_WMM_PKT_CLASSIFY_BASIS_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_DIR_AC_VO_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraDirAcVo, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_VO_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_VO_MIN, 
                 CFG_QOS_WMM_INFRA_DIR_AC_VO_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraNomMsduSizeAcVo, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MIN, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VO_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMeanDataRateAcVo, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MIN, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VO_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMinPhyRateAcVo, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MIN, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VO_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_SBA_AC_VO_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraSbaAcVo, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_VO_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_VO_MIN, 
                 CFG_QOS_WMM_INFRA_SBA_AC_VO_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_DIR_AC_VI_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraDirAcVi, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_VI_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_VI_MIN, 
                 CFG_QOS_WMM_INFRA_DIR_AC_VI_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraNomMsduSizeAcVi, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MIN, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_VI_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMeanDataRateAcVi, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MIN, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_VI_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMinPhyRateAcVi, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MIN, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_VI_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_SBA_AC_VI_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraSbaAcVi, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_VI_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_VI_MIN, 
                 CFG_QOS_WMM_INFRA_SBA_AC_VI_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_DIR_AC_BE_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraDirAcBe, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_BE_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_BE_MIN, 
                 CFG_QOS_WMM_INFRA_DIR_AC_BE_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraNomMsduSizeAcBe, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MIN, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BE_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMeanDataRateAcBe, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MIN, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BE_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMinPhyRateAcBe, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MIN, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BE_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_SBA_AC_BE_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraSbaAcBe, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_BE_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_BE_MIN, 
                 CFG_QOS_WMM_INFRA_SBA_AC_BE_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_DIR_AC_BK_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, InfraDirAcBk, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_BK_DEFAULT, 
                 CFG_QOS_WMM_INFRA_DIR_AC_BK_MIN, 
                 CFG_QOS_WMM_INFRA_DIR_AC_BK_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraNomMsduSizeAcBk, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_DEFAULT, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MIN, 
                 CFG_QOS_WMM_INFRA_NOM_MSDU_SIZE_AC_BK_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMeanDataRateAcBk, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MIN, 
                 CFG_QOS_WMM_INFRA_MEAN_DATA_RATE_AC_BK_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraMinPhyRateAcBk, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_DEFAULT, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MIN, 
                 CFG_QOS_WMM_INFRA_MIN_PHY_RATE_AC_BK_MAX ),

   REG_VARIABLE( CFG_QOS_WMM_INFRA_SBA_AC_BK_NAME , WLAN_PARAM_HexInteger,
                 hdd_config_t, InfraSbaAcBk, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_BK_DEFAULT, 
                 CFG_QOS_WMM_INFRA_SBA_AC_BK_MIN, 
                 CFG_QOS_WMM_INFRA_SBA_AC_BK_MAX ),

   REG_VARIABLE( CFG_TL_WFQ_BK_WEIGHT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, WfqBkWeight, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_TL_WFQ_BK_WEIGHT_DEFAULT, 
                 CFG_TL_WFQ_BK_WEIGHT_MIN, 
                 CFG_TL_WFQ_BK_WEIGHT_MAX ),

   REG_VARIABLE( CFG_TL_WFQ_BE_WEIGHT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, WfqBeWeight, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_TL_WFQ_BE_WEIGHT_DEFAULT, 
                 CFG_TL_WFQ_BE_WEIGHT_MIN, 
                 CFG_TL_WFQ_BE_WEIGHT_MAX ),

   REG_VARIABLE( CFG_TL_WFQ_VI_WEIGHT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, WfqViWeight, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_TL_WFQ_VI_WEIGHT_DEFAULT, 
                 CFG_TL_WFQ_VI_WEIGHT_MIN, 
                 CFG_TL_WFQ_VI_WEIGHT_MAX ),

   REG_VARIABLE( CFG_TL_WFQ_VO_WEIGHT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, WfqVoWeight, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_TL_WFQ_VO_WEIGHT_DEFAULT, 
                 CFG_TL_WFQ_VO_WEIGHT_MIN, 
                 CFG_TL_WFQ_VO_WEIGHT_MAX ),

   REG_VARIABLE( CFG_TL_DELAYED_TRGR_FRM_INT_NAME , WLAN_PARAM_Integer,
                 hdd_config_t, DelayedTriggerFrmInt, 
                 VAR_FLAGS_OPTIONAL | VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT, 
                 CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT, 
                 CFG_TL_DELAYED_TRGR_FRM_INT_MIN, 
                 CFG_TL_DELAYED_TRGR_FRM_INT_MAX ),

   REG_VARIABLE_STRING( CFG_WOWL_PATTERN_NAME, WLAN_PARAM_String,
                        hdd_config_t, wowlPattern, 
                        VAR_FLAGS_OPTIONAL,
                        (void *)CFG_WOWL_PATTERN_DEFAULT ),
};                                

/*
 * This function returns a pointer to the character after the occurence
 * of a new line character. It also modifies the original string by replacing
 * the '\n' character with the null character.
 * Function returns NULL if no new line character was found before end of
 * string was reached
 */
static char* get_next_line(char* str)
{
  char c;

  if( str == NULL || *str == '\0') {
    return NULL;
  }

  c = *str;
  while(c != '\n'  && c != '\0' && c != 0xd)  { 
    str = str + 1;
    c = *str;
  }

  if (c == '\0' ) {
    return NULL;
  }
  else
  {
    *str = '\0'; 
    return (str+1);
  }

  return NULL;
}

#define i_isspace(ch)  ((ch) == 0 || ((ch) >= 0x09 && (ch) <= 0x0d) || (ch) == ' ')

/*
 * This function trims any leading and trailing white spaces
 */
static char *i_trim(char *str)

{
   char *ptr;

   if(*str == '\0') return str;

   /* Find the first non white-space*/
   for (ptr = str; i_isspace(*ptr); ptr++);
      if (*ptr == '\0')
         return str;

   /* This is the new start of the string*/
   str = ptr;

   /* Find the last non white-space */
   ptr += strlen(ptr) - 1;
   for (; ptr != str && i_isspace(*ptr); ptr--);
      /* Null terminate the following character */
      ptr[1] = '\0';
                                  
   return str;
}


//Structure to store each entry in qcom_cfg.ini file
typedef struct
{
   char *name;
   char *value;
}tCfgIniEntry;

static VOS_STATUS hdd_apply_cfg_ini( hdd_adapter_t * pAdapter, 
    tCfgIniEntry* iniTable, unsigned long entries);

#ifdef WLAN_CFG_DEBUG
void dump_cfg_ini (tCfgIniEntry* iniTable, unsigned long entries) 
{
   unsigned long i;

   for (i = 0; i < entries; i++) {
       printk(KERN_ERR "qcom_cfg.ini entry Name=[%s] value=[%s]\n", 
           iniTable[i].name, iniTable[i].value);
     }
}
#endif 

/*
 * This function reads the qcom_cfg.ini file and
 * parses each 'Name=Value' pair in the ini file
 */
VOS_STATUS hdd_parse_config_ini(hdd_adapter_t* pAdapter)
{
   int status, i=0;
   /** Pointer for firmware image data */
   const struct firmware *fw;
   char *buffer, *line;
   size_t size;
   char *name, *value;
   tCfgIniEntry cfgIniTable[MAX_CFG_INI_ITEMS];
   VOS_STATUS vos_status = VOS_STATUS_SUCCESS;

   memset(cfgIniTable, 0, sizeof(cfgIniTable));

   status = request_firmware(&fw, "wlan/qcom_cfg.ini", &pAdapter->hsdio_func_dev->dev);
   
   if(!fw || !fw->data) {
      hddLog(LOGE,"%s: qcom_cfg.ini download failed\n",__FUNCTION__);
	    return VOS_STATUS_E_FAILURE;
   } 

   buffer = (char *)fw->data;
   size = fw->size;
  
   while (buffer != NULL)
   {

      line = get_next_line(buffer);
      buffer = i_trim(buffer);

      if(strlen((char*)buffer) == 0 || *buffer == '#')  {
         buffer = line;
         continue;
      }
      else if(strncmp(buffer, "END", 3) == 0 ) {
         break;
      }
      else
      {
         name = buffer;
         while(*buffer != '=' && *buffer != '\0') 
            buffer++;
         if(*buffer != '\0') {
            *buffer++ = '\0';
            i_trim(name);
            if(strlen (name) != 0) {
               buffer = i_trim(buffer);
               if(strlen(buffer)>0) {
                  value = buffer;
                  while(!i_isspace(*buffer) && *buffer != '\0') 
                     buffer++;
                  *buffer = '\0';
                  cfgIniTable[i].name= name;
                  cfgIniTable[i++].value= value;
                  if(i >= MAX_CFG_INI_ITEMS) {
                     hddLog(LOG1,"%s: Number of items in qcom_cfg.ini > %d \n",
                        __FUNCTION__, MAX_CFG_INI_ITEMS);
                     break;
                  }
               }
            }
         }
      }
      buffer = line;
   }

   //Loop through the registry table and apply all these configs
   vos_status = hdd_apply_cfg_ini(pAdapter, cfgIniTable, i);

   release_firmware(fw);
   return vos_status;
} 

void print_hdd_cfg(hdd_adapter_t *pAdapter)
{
  printk(KERN_ERR "*********Config values in HDD Adapter*******\n");
  printk(KERN_ERR "Name = [RTSThreshold] Value = %lu\n",pAdapter->cfg_ini->RTSThreshold) ;
  printk(KERN_ERR "Name = [OperatingChannel] Value = [%u]\n",pAdapter->cfg_ini->OperatingChannel);
  printk(KERN_ERR "Name = [PowerUsageControl] Value = [%s]\n",pAdapter->cfg_ini->PowerUsageControl);
  printk(KERN_ERR "Name = [fIsImpsEnabled] Value = [%u]\n",pAdapter->cfg_ini->fIsImpsEnabled);
  printk(KERN_ERR "Name = [AutoBmpsTimerEnabled] Value = [%u]\n",pAdapter->cfg_ini->fIsAutoBmpsTimerEnabled);
  printk(KERN_ERR "Name = [nAutoBmpsTimerValue] Value = [%lu]\n",pAdapter->cfg_ini->nAutoBmpsTimerValue);
  printk(KERN_ERR "Name = [nVccRssiTrigger] Value = [%u]\n",pAdapter->cfg_ini->nVccRssiTrigger);
  printk(KERN_ERR "Name = [gIbssBssid] Value =[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n",
      pAdapter->cfg_ini->IbssBssid.bytes[0],pAdapter->cfg_ini->IbssBssid.bytes[1],
      pAdapter->cfg_ini->IbssBssid.bytes[2],pAdapter->cfg_ini->IbssBssid.bytes[3],
      pAdapter->cfg_ini->IbssBssid.bytes[4],pAdapter->cfg_ini->IbssBssid.bytes[5]);
  printk(KERN_ERR "Name = [gStaMacAddr] Value =[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n",
      pAdapter->cfg_ini->staMacAddr.bytes[0],pAdapter->cfg_ini->staMacAddr.bytes[1],
      pAdapter->cfg_ini->staMacAddr.bytes[2],pAdapter->cfg_ini->staMacAddr.bytes[3],
      pAdapter->cfg_ini->staMacAddr.bytes[4],pAdapter->cfg_ini->staMacAddr.bytes[5]);
  printk(KERN_ERR "Name = [ChannelBondingMode] Value = [%lu]\n",pAdapter->cfg_ini->ChannelBondingMode);
  printk(KERN_ERR "Name = [ChannelBondingMode] Value = [%lu]\n",pAdapter->cfg_ini->ChannelBondingMode);
  printk(KERN_ERR "Name = [dot11Mode] Value = [%lu]\n",pAdapter->cfg_ini->dot11Mode);
  printk(KERN_ERR "Name = [WmmMode] Value = [%u]\n",pAdapter->cfg_ini->WmmMode);
  printk(KERN_ERR "Name = [UapsdMask] Value = [0x%x]\n",pAdapter->cfg_ini->UapsdMask);
  printk(KERN_ERR "Name = [PktClassificationBasis] Value = [%u]\n",pAdapter->cfg_ini->PktClassificationBasis);

  printk(KERN_ERR "Name = [InfraUapsdVoSrvIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdVoSrvIntv);
  printk(KERN_ERR "Name = [InfraUapsdVoSuspIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdVoSuspIntv);

  printk(KERN_ERR "Name = [InfraUapsdViSrvIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdViSrvIntv);
  printk(KERN_ERR "Name = [InfraUapsdViSuspIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdViSuspIntv);

  printk(KERN_ERR "Name = [InfraUapsdBeSrvIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdBeSrvIntv);
  printk(KERN_ERR "Name = [InfraUapsdBeSuspIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdBeSuspIntv);

  printk(KERN_ERR "Name = [InfraUapsdBkSrvIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdBkSrvIntv);
  printk(KERN_ERR "Name = [InfraUapsdBkSuspIntv] Value = [%lu]\n",pAdapter->cfg_ini->InfraUapsdBkSuspIntv);

  printk(KERN_ERR "Name = [InfraDirAcVo] Value = [%u]\n",pAdapter->cfg_ini->InfraDirAcVo);
  printk(KERN_ERR "Name = [InfraNomMsduSizeAcVo] Value = [0x%x]\n",pAdapter->cfg_ini->InfraNomMsduSizeAcVo);
  printk(KERN_ERR "Name = [InfraMeanDataRateAcVo] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMeanDataRateAcVo);
  printk(KERN_ERR "Name = [InfraMinPhyRateAcVo] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMinPhyRateAcVo);
  printk(KERN_ERR "Name = [InfraSbaAcVo] Value = [0x%x]\n",pAdapter->cfg_ini->InfraSbaAcVo);

  printk(KERN_ERR "Name = [InfraDirAcVi] Value = [%u]\n",pAdapter->cfg_ini->InfraDirAcVi);
  printk(KERN_ERR "Name = [InfraNomMsduSizeAcVi] Value = [0x%x]\n",pAdapter->cfg_ini->InfraNomMsduSizeAcVi);
  printk(KERN_ERR "Name = [InfraMeanDataRateAcVi] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMeanDataRateAcVi);
  printk(KERN_ERR "Name = [InfraMinPhyRateAcVi] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMinPhyRateAcVi);
  printk(KERN_ERR "Name = [InfraSbaAcVi] Value = [0x%x]\n",pAdapter->cfg_ini->InfraSbaAcVi);

  printk(KERN_ERR "Name = [InfraDirAcBe] Value = [%u]\n",pAdapter->cfg_ini->InfraDirAcBe);
  printk(KERN_ERR "Name = [InfraNomMsduSizeAcBe] Value = [0x%x]\n",pAdapter->cfg_ini->InfraNomMsduSizeAcBe);
  printk(KERN_ERR "Name = [InfraMeanDataRateAcBe] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMeanDataRateAcBe);
  printk(KERN_ERR "Name = [InfraMinPhyRateAcBe] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMinPhyRateAcBe);
  printk(KERN_ERR "Name = [InfraSbaAcBe] Value = [0x%x]\n",pAdapter->cfg_ini->InfraSbaAcBe);

  printk(KERN_ERR "Name = [InfraDirAcBk] Value = [%u]\n",pAdapter->cfg_ini->InfraDirAcBk);
  printk(KERN_ERR "Name = [InfraNomMsduSizeAcBk] Value = [0x%x]\n",pAdapter->cfg_ini->InfraNomMsduSizeAcBk);
  printk(KERN_ERR "Name = [InfraMeanDataRateAcBk] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMeanDataRateAcBk);
  printk(KERN_ERR "Name = [InfraMinPhyRateAcBk] Value = [0x%lx]\n",pAdapter->cfg_ini->InfraMinPhyRateAcBk);
  printk(KERN_ERR "Name = [InfraSbaAcBk] Value = [0x%x]\n",pAdapter->cfg_ini->InfraSbaAcBk);

  printk(KERN_ERR "Name = [WfqBkWeight] Value = [%u]\n",pAdapter->cfg_ini->WfqBkWeight);
  printk(KERN_ERR "Name = [WfqBeWeight] Value = [%u]\n",pAdapter->cfg_ini->WfqBeWeight);
  printk(KERN_ERR "Name = [WfqViWeight] Value = [%u]\n",pAdapter->cfg_ini->WfqViWeight);
  printk(KERN_ERR "Name = [WfqVoWeight] Value = [%u]\n",pAdapter->cfg_ini->WfqVoWeight);
  printk(KERN_ERR "Name = [DelayedTriggerFrmInt] Value = [%lu]\n",pAdapter->cfg_ini->DelayedTriggerFrmInt);

}


static VOS_STATUS find_cfg_item (tCfgIniEntry* iniTable, unsigned long entries,
    char *name, char** value) 
{
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   unsigned long i;

   for (i = 0; i < entries; i++) {
     if (strcmp(iniTable[i].name, name) == 0) {
       *value = iniTable[i].value;
       printk(KERN_ERR "Found qcom_cfg.ini entry for Name=[%s] Value=[%s]\n",
           name, *value);
       return VOS_STATUS_SUCCESS;
     }
   }

   return status;
}

static int parseHexDigit(char c)
{
  if (c >= '0' && c <= '9')
    return c-'0';
  if (c >= 'a' && c <= 'f')
    return c-'a'+10;
  if (c >= 'A' && c <= 'F')
    return c-'A'+10;

  return 0;
}


static VOS_STATUS hdd_apply_cfg_ini( hdd_adapter_t *pAdapter, tCfgIniEntry* iniTable, unsigned long entries)
{
   VOS_STATUS match_status = VOS_STATUS_E_FAILURE;
   VOS_STATUS ret_status = VOS_STATUS_SUCCESS;
   unsigned int idx;
   void *pField;
   char *value_str = NULL;
   unsigned long len_value_str; 
   char *candidate;
   v_U32_t value;
   void *pStructBase = pAdapter->cfg_ini;
   REG_TABLE_ENTRY *pRegEntry = g_registry_table;
   unsigned long cRegTableEntries  = sizeof(g_registry_table) / sizeof( g_registry_table[ 0 ]);
   v_U32_t cbOutString;
   int i;

   for ( idx = 0; idx < cRegTableEntries; idx++, pRegEntry++ ) 
   {
      //Calculate the address of the destination field in the structure.
      pField = ( (v_U8_t *)pStructBase )+ pRegEntry->VarOffset;

      match_status = find_cfg_item(iniTable, entries, pRegEntry->RegName, &value_str); 

      if( (match_status != VOS_STATUS_SUCCESS) && ( pRegEntry->Flags & VAR_FLAGS_REQUIRED ) )
      {
         // If we could not read the cfg item and it is required, this is an error.
         hddLog(LOG1, "%s: Failed to read required config parameter %s", 
            __FUNCTION__, pRegEntry->RegName);
         ret_status = VOS_STATUS_E_FAILURE;
         break;
      }

      if ( ( WLAN_PARAM_Integer    == pRegEntry->RegType ) ||
           ( WLAN_PARAM_HexInteger == pRegEntry->RegType ) ) 
      {
         // If successfully read from the registry, use the value read.  
         // If not, use the default value.
         if ( match_status == VOS_STATUS_SUCCESS && (WLAN_PARAM_Integer == pRegEntry->RegType)) {
            value = simple_strtoul(value_str, NULL, 10);
         }
         else if ( match_status == VOS_STATUS_SUCCESS && (WLAN_PARAM_HexInteger == pRegEntry->RegType)) {
            value = simple_strtoul(value_str, NULL, 16);
         }
         else {
            value = pRegEntry->VarDefault;
         }

         // If this parameter needs range checking, do it here.
         if ( pRegEntry->Flags & VAR_FLAGS_RANGE_CHECK ) 
         {
            if ( value > pRegEntry->VarMax )
            {
                hddLog(LOG1, "%s: Reg Parameter %s > allowed Maximum [%lu > %lu]. Enforcing Maximum\n", 
                   __FUNCTION__, pRegEntry->RegName, value, pRegEntry->VarMax );
                  value = pRegEntry->VarMax;
            }

            if ( value < pRegEntry->VarMin ) 
            {
                 hddLog(LOG1, "%s: Reg Parameter %s < allowed Minimum [%lu < %lu]. Enforcing Minimum", 
                    __FUNCTION__, pRegEntry->RegName, value, pRegEntry->VarMin);
                  value = pRegEntry->VarMin;
            } 
         }
         // If this parameter needs range checking, do it here.
         else if ( pRegEntry->Flags & VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT ) 
         {
            if ( value > pRegEntry->VarMax ) 
            {
               hddLog(LOG1, "%s: Reg Parameter %s > allowed Maximum [%lu > %lu]. Enforcing Default= %lu", 
                  __FUNCTION__, pRegEntry->RegName, value, pRegEntry->VarMax, pRegEntry->VarDefault  );
               value = pRegEntry->VarDefault;
            }

            if ( value < pRegEntry->VarMin ) 
            {
               hddLog(LOG1, "%s:Reg Parameter %s < allowed Minimum [%lu < %lu]. Enforcing Default= %lu", 
                  __FUNCTION__, pRegEntry->RegName, value, pRegEntry->VarMin, pRegEntry->VarDefault  );
               value = pRegEntry->VarDefault;
            } 
         }

         // Move the variable into the output field.
         memcpy( pField, &value, pRegEntry->VarSize );
      }
      // Handle string parameters
      else if ( WLAN_PARAM_String == pRegEntry->RegType )
      {
#ifdef WLAN_CFG_DEBUG
         printk(KERN_ERR "RegName = %s, VarOffset %u VarSize %u VarDefault %s\n",
            pRegEntry->RegName, pRegEntry->VarOffset, pRegEntry->VarSize, (char*)pRegEntry->VarDefault); 
#endif    

         if ( match_status == VOS_STATUS_SUCCESS) 
         {
            len_value_str = strlen(value_str);

            if(len_value_str > (pRegEntry->VarSize - 1)) {
               hddLog(LOG1, "%s: Invalid Value=[%s] specified for Name=[%s] in qcom_cfg.ini\n", 
                  __FUNCTION__, value_str, pRegEntry->RegName);
               cbOutString = utilMin( strlen( (char *)pRegEntry->VarDefault ), pRegEntry->VarSize - 1 );
               memcpy( pField, (void *)(pRegEntry->VarDefault), cbOutString );
               ( (v_U8_t *)pField )[ cbOutString ] = '\0';
            }
            else
            {
               memcpy( pField, (void *)(value_str), len_value_str);
               ( (v_U8_t *)pField )[ len_value_str ] = '\0';
            }
         }
         else 
         {
            // Failed to read the string parameter from the registry.  Use the default.
            cbOutString = utilMin( strlen( (char *)pRegEntry->VarDefault ), pRegEntry->VarSize - 1 );
            memcpy( pField, (void *)(pRegEntry->VarDefault), cbOutString );
            ( (v_U8_t *)pField )[ cbOutString ] = '\0';                 
         }
      }
      else if ( WLAN_PARAM_MacAddr == pRegEntry->RegType )
      {
         if(pRegEntry->VarSize != VOS_MAC_ADDR_SIZE) {
               hddLog(LOG1, "%s: Invalid VarSize %u for Name=[%s]\n", 
                   __FUNCTION__, pRegEntry->VarSize, pRegEntry->RegName); 
            continue;
         }
         candidate = (char*)pRegEntry->VarDefault;
         if ( match_status == VOS_STATUS_SUCCESS) {
            len_value_str = strlen(value_str);
            if(len_value_str != (VOS_MAC_ADDR_SIZE*2)) {
               hddLog(LOG1, "%s: Invalid MAC addr [%s] specified for Name=[%s] in qcom_cfg.ini\n", 
                  __FUNCTION__, value_str, pRegEntry->RegName);
            }
            else
               candidate = value_str;
         }
         //parse the string and store it in the byte array
         for(i=0; i<VOS_MAC_ADDR_SIZE; i++)
         {
            ((char*)pField)[i] = 
              (char)(parseHexDigit(candidate[i*2])*16 + parseHexDigit(candidate[i*2+1])); 
         }
      }
      else
      {
         hddLog(LOG1, "%s: Unknown param type for name[%s] in registry table\n", 
            __FUNCTION__, pRegEntry->RegName);
      }

   }

   print_hdd_cfg(pAdapter);

  return( ret_status );
}

typedef enum
{
     eHDD_DOT11_MODE_AUTO = 0, //Taurus mean everything because it covers all thing we support
     eHDD_DOT11_MODE_abg,//11a/b/g only, no HT, no proprietary
     eHDD_DOT11_MODE_11b,
     eHDD_DOT11_MODE_11g,
     eHDD_DOT11_MODE_11n,
     eHDD_DOT11_MODE_11g_ONLY,
     eHDD_DOT11_MODE_11n_ONLY,
     eHDD_DOT11_MODE_11b_ONLY,
}eHddDot11Mode;

static void hdd_xlate_to_csr_phy_mode( hdd_config_t *pConfig )
{
   if (pConfig == NULL) 
   {
      return;
   }

   switch (pConfig->dot11Mode) 
   {
      case (eHDD_DOT11_MODE_abg):
         pConfig->dot11Mode = eCSR_DOT11_MODE_abg;
         break;
      case (eHDD_DOT11_MODE_11b):
         pConfig->dot11Mode = eCSR_DOT11_MODE_11b;
         break;
      case (eHDD_DOT11_MODE_11g):
         pConfig->dot11Mode = eCSR_DOT11_MODE_11g;
         break;
      case (eHDD_DOT11_MODE_11n):
         pConfig->dot11Mode = eCSR_DOT11_MODE_11n;
         break;
      case (eHDD_DOT11_MODE_11g_ONLY):
         pConfig->dot11Mode = eCSR_DOT11_MODE_11g_ONLY;
         break;
      case (eHDD_DOT11_MODE_11n_ONLY):
         pConfig->dot11Mode = eCSR_DOT11_MODE_11n_ONLY;
         break;
      case (eHDD_DOT11_MODE_11b_ONLY):
         pConfig->dot11Mode = eCSR_DOT11_MODE_11b_ONLY;
         break;
      case (eHDD_DOT11_MODE_AUTO):
         pConfig->dot11Mode = eCSR_DOT11_MODE_AUTO;
         break;
      default:
         break;
   }

}

static void hdd_set_power_save_config(hdd_adapter_t *pAdapter, tSmeConfigParams *smeConfig) 
{
   hdd_config_t *pConfig = pAdapter->cfg_ini;

   tPmcBmpsConfigParams bmpsParams;
   
   sme_GetConfigPowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE, &bmpsParams);
   
   if (strcmp(pConfig->PowerUsageControl, "Min") == 0)
   {
      smeConfig->csrConfig.impsSleepTime   = pConfig->nImpsMinSleepTime;
      bmpsParams.bmpsPeriod                = pConfig->nBmpsMinListenInterval;
   }
   if (strcmp(pConfig->PowerUsageControl, "Max") == 0)
   {
      smeConfig->csrConfig.impsSleepTime   = pConfig->nImpsMaxSleepTime;
      bmpsParams.bmpsPeriod                = pConfig->nBmpsMaxListenInterval;
   }
   if (strcmp(pConfig->PowerUsageControl, "Mod") == 0)
   {
      smeConfig->csrConfig.impsSleepTime   = pConfig->nImpsModSleepTime;
      bmpsParams.bmpsPeriod                = pConfig->nBmpsModListenInterval;
   }

   if (pConfig->fIsImpsEnabled)
   {
      sme_EnablePowerSave (pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   }
   else
   {
      sme_DisablePowerSave (pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   }

   if (pConfig->fIsBmpsEnabled)
   {
      sme_EnablePowerSave (pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   }
   else
   {
      sme_DisablePowerSave (pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   }

   bmpsParams.trafficMeasurePeriod = pConfig->nAutoBmpsTimerValue;

   if (sme_SetConfigPowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE, &bmpsParams)== eHAL_STATUS_FAILURE)
   {
      hddLog(LOG1, "SetConfigPowerSave failed to set BMPS params\n");
   }
  
   if(pConfig->fIsAutoBmpsTimerEnabled)
   {
      sme_StartAutoBmpsTimer(pAdapter->hHal);
   }

}

#ifdef FEATURE_WLAN_GEN6_ROAMING

static VOS_STATUS hdd_get_ho_config ( hdd_config_t *pConfig, tSmeHoConfigParams *smeHoCfg )
{

   //No Wifi Params
   smeHoCfg->csrHoConfig.noWifiParams.rssiFilterConst = pConfig->nRssiFilterConstNoWifi;
   smeHoCfg->csrHoConfig.noWifiParams.channelScanTime = pConfig->nChannelScanTime;
   smeHoCfg->csrHoConfig.noWifiParams.rssiThresholdNeighborSet = pConfig->nRssiThreshNeigh;
   smeHoCfg->csrHoConfig.noWifiParams.rssiThresholdAssociationAdd = pConfig->nRssiThreshAssoc;
   smeHoCfg->csrHoConfig.noWifiParams.activeScanInterval = pConfig->nActiveScanInterval;
   smeHoCfg->csrHoConfig.noWifiParams.activeScanDuration = pConfig->nActiveScanDuration;

   //No Traffic Params
   smeHoCfg->csrHoConfig.ntParams.rssiFilterConst = pConfig->nRssiFilterConstNt;
   smeHoCfg->csrHoConfig.ntParams.numCandtSetEntry = pConfig->nNumCandidateSetNt;
   smeHoCfg->csrHoConfig.ntParams.inactThreshold = pConfig->nInactThreshNt;
   smeHoCfg->csrHoConfig.ntParams.inactPeriod = pConfig->nInactPeriodNt;
   smeHoCfg->csrHoConfig.ntParams.bestCandidateApRssiDelta = pConfig->nBestCandidateApRssiDeltaNt;
   smeHoCfg->csrHoConfig.ntParams.neighborApBgScanInterval = pConfig->nNeighApBgScanIntervalNt;
   smeHoCfg->csrHoConfig.ntParams.neighborApIncrBgScanInterval = pConfig->nNeighApIncrNt;
   smeHoCfg->csrHoConfig.ntParams.rssiThresholdCandtSet = pConfig->nRssiThreshCandidateNt;
   smeHoCfg->csrHoConfig.ntParams.pmkCacheRssiDelta = pConfig->nPmkCacheRssiNt;
   smeHoCfg->csrHoConfig.ntParams.rssiThresholdCurrentApGood = pConfig->nRssiThresholdCurrentApGoodNt;

   //Non Real Time Params
   smeHoCfg->csrHoConfig.nrtParams.rssiFilterConst = pConfig->nRssiFilterConstNrt;
   smeHoCfg->csrHoConfig.nrtParams.numCandtSetEntry= pConfig->nNumCandtSetEntryNrt; 
   smeHoCfg->csrHoConfig.nrtParams.rssiThresholdCurrentApGood = pConfig->nRssiThresholdCurrentApGoodNrt;
   smeHoCfg->csrHoConfig.nrtParams.rssiThresholdCurrentApGoodEmptyCandtset = 
     pConfig->nRssiThresholdCurrentApGoodEmptyCandtsetNrt;
   smeHoCfg->csrHoConfig.nrtParams.rssiThresholdHoFromCurrentAp = pConfig->nRssiThresholdHoFromCurrentApNrt;
   smeHoCfg->csrHoConfig.nrtParams.rssiThresholdCandtSet = pConfig->nRssiThresholdCandtSetNrt;
   smeHoCfg->csrHoConfig.nrtParams.bgScanInterval = pConfig->nBgScanIntervalNrt;
   smeHoCfg->csrHoConfig.nrtParams.bgScanIncrInterval = pConfig->nBgScanIncrIntervalNrt;
   smeHoCfg->csrHoConfig.nrtParams.bgScanDelayInterval = pConfig->nBgScanDelayIntervalNrt;
   smeHoCfg->csrHoConfig.nrtParams.perMsmtInterval = pConfig->nPerMsmtIntervalNrt;
   smeHoCfg->csrHoConfig.nrtParams.perThresholdHoFromCurrentAp = pConfig->nPerThresholdHoFromCurrentApNrt;
   smeHoCfg->csrHoConfig.nrtParams.pmkCacheRssiDelta = pConfig->nPmkCacheRssiDeltaNrt;
   smeHoCfg->csrHoConfig.nrtParams.bestCandidateApRssiDelta = pConfig->nBestCandidateApRssiDeltaNrt;

   // Real time params
   smeHoCfg->csrHoConfig.rtParams.rssiFilterConst = pConfig->nRssiFilterConstRt;
   smeHoCfg->csrHoConfig.rtParams.numCandtSetEntry = pConfig->nNumCandtSetEntryRt;
   smeHoCfg->csrHoConfig.rtParams.rssiThresholdCurrentApGood = pConfig->nRssiThresholdCurrentApGoodRt;
   smeHoCfg->csrHoConfig.rtParams.rssiThresholdHoFromCurrentAp = pConfig->nRssiThresholdHoFromCurrentApRt;
   smeHoCfg->csrHoConfig.rtParams.rssiThresholdCandtSet = pConfig->nRssiThresholdCandtSetRt;
   smeHoCfg->csrHoConfig.rtParams.bgScanInterval = pConfig->nBgScanIntervalRt;
   smeHoCfg->csrHoConfig.rtParams.perMsmtInterval = pConfig->nPerMsmtIntervalRt;
   smeHoCfg->csrHoConfig.rtParams.perThresholdHoFromCurrentAp = pConfig->nPerThresholdHoFromCurrentApRt;
   smeHoCfg->csrHoConfig.rtParams.pmkCacheRssiDelta = pConfig->nPmkCacheRssiDeltaRt;
   smeHoCfg->csrHoConfig.rtParams.bestCandidateApRssiDelta = pConfig->nBestCandidateApRssiDeltaRt;

   return VOS_STATUS_SUCCESS;
}
#endif //FEATURE_WLAN_GEN6_ROAMING

v_BOOL_t hdd_update_config_dat( hdd_adapter_t *pAdapter )
{
   v_BOOL_t  fStatus = TRUE;
   v_U32_t   value;

   hdd_config_t *pConfig = pAdapter->cfg_ini;

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_SHORT_GI_20MHZ, 
      pConfig->ShortGI20MhzEnable, NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_SHORT_GI_20MHZ to CCM\n");
   }
       
   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_CAL_CONTROL, pConfig->Calibration, 
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_CAL_CONTROL to CCM\n");
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_CAL_PERIOD, pConfig->CalibrationPeriod,
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_CAL_PERIOD to CCM\n");
   }

   if ( 0 != pConfig->Cfg1Id )
   {
      if (ccmCfgSetInt(pAdapter->hHal, pConfig->Cfg1Id, pConfig->Cfg1Value, NULL, 
         eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1, "Could not pass on Cfg1Id to CCM\n");
      }
          
   }

   if ( 0 != pConfig->Cfg2Id )
   {
      if (ccmCfgSetInt(pAdapter->hHal, pConfig->Cfg2Id, pConfig->Cfg2Value, 
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1, "Could not pass on Cfg2Id to CCM\n");
      }
   }

   if ( 0 != pConfig->Cfg3Id )
   {
      if (ccmCfgSetInt(pAdapter->hHal, pConfig->Cfg3Id, pConfig->Cfg3Value, 
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1, "Could not pass on Cfg3Id to CCM\n");
      }
   }

   if ( 0 != pConfig->Cfg4Id )
   {
      if (ccmCfgSetInt(pAdapter->hHal, pConfig->Cfg4Id, pConfig->Cfg4Value, 
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1, "Could not pass on Cfg4Id to CCM\n");
      }
   }

   if ( 0 != pConfig->Cfg5Id )
   {
      if (ccmCfgSetInt(pAdapter->hHal, pConfig->Cfg5Id, pConfig->Cfg5Value, 
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1, "Could not pass on Cfg5Id to CCM\n");
      }
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_BA_AUTO_SETUP, pConfig->BlockAckAutoSetup, 
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_BA_AUTO_SETUP to CCM\n");
   }
       
   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_FIXED_RATE, pConfig->TxRate, 
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_FIXED_RATE to CCM\n");
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_MAX_RX_AMPDU_FACTOR, 
      pConfig->MaxRxAmpduFactor, NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1,"Could not pass on WNI_CFG_HT_AMPDU_PARAMS_MAX_RX_AMPDU_FACTOR to CCM\n");
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_SHORT_PREAMBLE, pConfig->fIsShortPreamble,
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1,"Could not pass on WNI_CFG_SHORT_PREAMBLE to CCM\n");
   }

   if (pConfig->fIsAutoIbssBssid) 
   {
      if (ccmCfgSetStr(pAdapter->hHal, WNI_CFG_BSSID, (v_U8_t *)"000000000000", 
         sizeof(v_BYTE_t) * VOS_MAC_ADDR_SIZE, NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1,"Could not pass on WNI_CFG_BSSID to CCM\n");
      }
   }
   else
   { 
      if ( VOS_FALSE == vos_is_macaddr_group( &pConfig->IbssBssid ))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                    "MAC Addr (IBSS BSSID) read from Registry is: %02x-%02x-%02x-%02x-%02x-%02x",
                    pConfig->IbssBssid.bytes[0], pConfig->IbssBssid.bytes[1], pConfig->IbssBssid.bytes[2], 
                    pConfig->IbssBssid.bytes[3], pConfig->IbssBssid.bytes[4], pConfig->IbssBssid.bytes[5]);
         if (ccmCfgSetStr(pAdapter->hHal, WNI_CFG_BSSID, pConfig->IbssBssid.bytes, 
            sizeof(v_BYTE_t) * VOS_MAC_ADDR_SIZE, NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
         {
            fStatus = FALSE;
            hddLog(LOG1,"Could not pass on WNI_CFG_BSSID to CCM\n");
         }
      }
      else
      {
         fStatus = FALSE;
         hddLog(LOG1,"Could not pass on WNI_CFG_BSSID to CCM\n");
      }
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_BEACON_INTERVAL, pConfig->nBeaconInterval, 
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_BEACON_INTERVAL to CCM\n");
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_MAX_PS_POLL, pConfig->nMaxPsPoll, 
      NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
	  fStatus = FALSE;
	  hddLog(LOG1, "Could not pass on WNI_CFG_MAX_PS_POLL to CCM\n");
   }

   //Initially, firmware configures Libra to 2 Rx Antennas
   //this config triggers HAL to configure Rx Antenna accordingly
   if (ccmCfgGetInt(pAdapter->hHal, WNI_CFG_CURRENT_RX_ANTENNA, &value)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not read WNI_CFG_CURRENT_RX_ANTENNA cfg parameter\n");
   }
   else
   {
      if(ccmCfgSetInt(pAdapter->hHal, WNI_CFG_CURRENT_RX_ANTENNA, value, NULL, 
         eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
      {
         fStatus = FALSE;
         hddLog(LOG1, "Could not pass on WNI_CFG_CURRENT_RX_ANTENNA to CCM\n");

      }
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_LOW_GAIN_OVERRIDE, pConfig->fIsLowGainOverride, 
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_LOW_GAIN_OVERRIDE to HAL\n");
   }
 
   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_RSSI_FILTER_PERIOD, pConfig->nRssiFilterPeriod,
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_CFG_RSSI_FILTER_PERIOD to CCM\n");
   }

   if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_IGNORE_DTIM, pConfig->fIgnoreDtim,
         NULL, eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
   {
      fStatus = FALSE;
      hddLog(LOG1, "Could not pass on WNI_IGNORE_DTIM configuration to CCM\n"  );
   }

   return fStatus;
}


/**---------------------------------------------------------------------------

  \brief hdd_init_set_sme_config() - 

   This function initializes the sme configuration parameters
   
  \param  - pAdapter - Pointer to the HDD Adapter.
              
  \return - 0 for success, non zero for failure
    
  --------------------------------------------------------------------------*/

VOS_STATUS hdd_set_sme_config( hdd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   eHalStatus halStatus;
   tSmeConfigParams smeConfig;
#ifdef FEATURE_WLAN_GEN6_ROAMING
   tSmeHoConfigParams smeHoCfg;
#endif
   hdd_config_t *pConfig = pAdapter->cfg_ini;

   vos_mem_zero( &smeConfig, sizeof( smeConfig ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, 
              "%s bWmmIsEnabled=%d 802_11e_enabled=%d dot11Mode=%d\n", __func__,
              pConfig->WmmMode, pConfig->b80211eIsEnabled, pConfig->dot11Mode);

   // Config params obtained from the registry
   smeConfig.csrConfig.RTSThreshold             = pConfig->RTSThreshold;
   smeConfig.csrConfig.FragmentationThreshold   = pConfig->FragmentationThreshold;
   smeConfig.csrConfig.shortSlotTime            = pConfig->ShortSlotTimeEnabled;
   smeConfig.csrConfig.Is11dSupportEnabled      = pConfig->Is11dSupportEnabled;
   smeConfig.csrConfig.HeartbeatThresh24        = pConfig->HeartbeatThresh24;

   hdd_xlate_to_csr_phy_mode ( pConfig );
   smeConfig.csrConfig.phyMode                  = pConfig->dot11Mode;

   smeConfig.csrConfig.ChannelBondingMode       = pConfig->ChannelBondingMode;
   smeConfig.csrConfig.TxRate                   = pConfig->TxRate;
   smeConfig.csrConfig.nScanResultAgeCount      = pConfig->ScanResultAgeCount;
   smeConfig.csrConfig.scanAgeTimeNCNPS         = pConfig->nScanAgeTimeNCNPS;
   smeConfig.csrConfig.scanAgeTimeNCPS          = pConfig->nScanAgeTimeNCPS;
   smeConfig.csrConfig.scanAgeTimeCNPS          = pConfig->nScanAgeTimeCNPS;
   smeConfig.csrConfig.scanAgeTimeCPS           = pConfig->nScanAgeTimeCPS;
   smeConfig.csrConfig.AdHocChannel24           = pConfig->OperatingChannel;
   smeConfig.csrConfig.fEnforce11dChannels      = pConfig->fEnforce11dChannels;
   smeConfig.csrConfig.fEnforceCountryCodeMatch = pConfig->fEnforceCountryCodeMatch;
   smeConfig.csrConfig.fEnforceDefaultDomain    = pConfig->fEnforceDefaultDomain;
   smeConfig.csrConfig.bCatRssiOffset           = pConfig->nRssiCatGap;
   smeConfig.csrConfig.vccRssiThreshold         = pConfig->nVccRssiTrigger;
   smeConfig.csrConfig.vccUlMacLossThreshold    = pConfig->nVccUlMacLossThreshold;
   smeConfig.csrConfig.nRoamingTime             = pConfig->nRoamingTime;
   smeConfig.csrConfig.IsIdleScanEnabled        = pConfig->nEnableIdleScan; 
   smeConfig.csrConfig.nActiveMaxChnTime        = pConfig->nActiveMaxChnTime;
   smeConfig.csrConfig.nActiveMinChnTime        = pConfig->nActiveMinChnTime;
   smeConfig.csrConfig.nPassiveMaxChnTime       = pConfig->nPassiveMaxChnTime;
   smeConfig.csrConfig.nPassiveMinChnTime       = pConfig->nPassiveMinChnTime;
   smeConfig.csrConfig.Is11eSupportEnabled      = pConfig->b80211eIsEnabled;
   smeConfig.csrConfig.WMMSupportMode           = pConfig->WmmMode;

   //Remaining config params not obtained from registry
   // On RF EVB beacon using channel 1.
   
   smeConfig.csrConfig.AdHocChannel5G            = 44; 
   smeConfig.csrConfig.ProprietaryRatesEnabled   = 0;  
   smeConfig.csrConfig.HeartbeatThresh50         = 40; 
   smeConfig.csrConfig.Is11hSupportEnabled       = 0; 
   smeConfig.csrConfig.bandCapability            = eCSR_BAND_24; 
   smeConfig.csrConfig.cbChoice                  = 0;   
   smeConfig.csrConfig.bgScanInterval            = 0; 
   smeConfig.csrConfig.eBand                     = eCSR_BAND_24; 
   smeConfig.csrConfig.nTxPowerCap = pConfig->nTxPowerCap;
   hdd_set_power_save_config(pAdapter, &smeConfig);

   halStatus = sme_UpdateConfig( pAdapter->hHal, &smeConfig );    
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      status = VOS_STATUS_E_FAILURE;
   }

#ifdef FEATURE_WLAN_GEN6_ROAMING   
   if (VOS_IS_STATUS_SUCCESS(hdd_get_ho_config ( pConfig, &smeHoCfg )))
   {
      halStatus = sme_HoConfig(pAdapter->hHal, &smeHoCfg);
      if ( !HAL_STATUS_SUCCESS( halStatus ) )
      {
		     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
		   		   "Passing HO config info from registry to SME failed");
         status = VOS_STATUS_E_FAILURE;
      }
   } 
#endif
   
   return status;   
}
