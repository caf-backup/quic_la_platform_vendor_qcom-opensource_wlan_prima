/*===========================================================================
  \file wlan_btc_usr_svc.h
  
  Exports and types for the BTC Service interface. This is shared between
  BTC-ES layer and BTC Services layer.
  
  This implementation is specific to Android platform and works with BM3
  Bluetooth Stack (runninng in user space)

  Copyright (c) 2009 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary

===========================================================================*/

#ifndef WLAN_BTC_USR_SVC_H__
#define WLAN_BTC_USR_SVC_H__

#include <btces.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * External Functions
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Type Declarations
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Function Prototypes
 *-------------------------------------------------------------------------*/
int btc_svc_init (btces_funcs *pBtcEsFuncs);
void btc_svc_deinit (void);

#ifdef __cplusplus
}
#endif

#endif //WLAN_BTC_USR_SVC_H__
