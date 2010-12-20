#ifndef __WLAN_HDD_POWER_H
#define __WLAN_HDD_POWER_H

/**===========================================================================

  \file  wlan_hdd_power.h

  \brief Linux HDD Power
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.

  ==========================================================================*/


/*--------------------------------------------------------------------------
 * Include Files
 *------------------------------------------------------------------------*/
#include "wlan_hdd_main.h"

/*---------------------------------------------------------------------------
 *   Preprocessor definitions and constants
 *-------------------------------------------------------------------------*/
 //gEnableSuspend = 1 in INI file implies suspend to standby
 #define WLAN_MAP_SUSPEND_TO_STANDBY     1

 //gEnableSuspend = 2 in INI file implies suspend to deep sleep
 #define WLAN_MAP_SUSPEND_TO_DEEP_SLEEP  2

 //gEnableSuspend = 3 in INI file implies suspend to set MCAST/BCAST filter 
 #define WLAN_MAP_SUSPEND_TO_MCAST_BCAST_FILTER  3

 //gEnableDriverStop = 1 implies map driver stop to standby
 #define WLAN_MAP_DRIVER_STOP_TO_STANDBY     1

 //gEnableDriverStop = 2 implies map sriver stop to deep sleep
 #define WLAN_MAP_DRIVER_STOP_TO_DEEP_SLEEP  2

 //Maximum time (ms) to wait for standby to complete
 #define WLAN_WAIT_TIME_STANDBY          1000

 //Maximum time (ms) to wait for full pwr to complete
 #define WLAN_WAIT_TIME_FULL_PWR         1000


/*---------------------------------------------------------------------------
 *   Type declarations
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Function declarations and documentation
 * ------------------------------------------------------------------------*/
 eHalStatus hdd_exit_standby(hdd_adapter_t* pAdapter);
 VOS_STATUS hdd_exit_deep_sleep(hdd_adapter_t* pAdapter);
 VOS_STATUS hdd_enter_standby(hdd_adapter_t* pAdapter);
 VOS_STATUS hdd_enter_deep_sleep(hdd_adapter_t* pAdapter);
 VOS_STATUS hdd_wlan_reset(void) ;
void hdd_conf_mcastbcast_filter(hdd_adapter_t* pAdapter, v_BOOL_t setfilter);



#endif // if !defined __WLAN_QCT_DRIVER_H
