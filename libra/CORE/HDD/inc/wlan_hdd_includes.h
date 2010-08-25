#if !defined( HDD_INCLUDES_H__ )
#define HDD_INCLUDES_H__

/**===========================================================================
  
  \file  wlan_hdd_includes.h
  
  \brief Internal includes for the Linux HDD 
  
               Copyright 2008 (c) Qualcomm, Incorporated.
               All Rights Reserved.
               Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/* $HEADER$ */
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 

// throw all the includes in here f to get the .c files  in the HDD to compile.

//This is wrong.  CONFIG_QCOM_WLAN_HAVE_NET_DEVICE_OPS shouldn't be used in
//this way. Use Linux version instead. Anything >= 2.6.31 behaves this way.
#ifdef USE_AUTOCONF_IN_GENERATED
#include <generated/autoconf.h>
#else  // ~USE_AUTOCONF_IN_GENERATED
#include <linux/autoconf.h>
#endif //USE_AUTOCONF_IN_GENERATED
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <linux/if_arp.h>


#include <vos_api.h>

#include <sme_Api.h>
#include <wlan_qct_tl.h>

#include "wlan_hdd_assoc.h"
#include "wlan_hdd_dp_utils.h"
#include "wlan_hdd_mib.h"
#include "wlan_hdd_wext.h"
#include "wlan_hdd_main.h"
#include "wlan_hdd_version.h"
#include "wlan_hdd_tx_rx.h"

#endif    // end #if !defined( HDD_INCLUDES_H__ )
