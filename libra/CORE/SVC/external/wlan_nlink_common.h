/*===========================================================================
  \file wlan_nlink_common.h
  
  Exports and types for the Netlink Service interface. This header file contains
  message types and definitions that is shared between the user space service
  (e.g. BTC service) and WLAN kernel module.

  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary

===========================================================================*/

#ifndef WLAN_NLINK_COMMON_H__
#define WLAN_NLINK_COMMON_H__

#include <linux/netlink.h>

/*---------------------------------------------------------------------------
 * External Functions
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-------------------------------------------------------------------------*/
#define WLAN_NL_MAX_PAYLOAD   256     /* maximum size for netlink message*/
#define WLAN_NLINK_PROTO_FAMILY  NETLINK_USERSOCK
#define WLAN_NLINK_MCAST_GRP_ID  0x01 

/*---------------------------------------------------------------------------
 * Type Declarations
 *-------------------------------------------------------------------------*/

/* 
 * The following enum defines the target service within WLAN driver for which the
 * message is intended for. Each service along with its counterpart 
 * in the user space, define a set of messages they recognize.
 * Each of this message will have an header of type tAniMsgHdr defined below.
 * Each Netlink message to/from a kernel module will contain only one
 * message which is preceded by a tAniMsgHdr. The maximun size (in bytes) of
 * a netlink message is assumed to be MAX_PAYLOAD bytes.
 *
 *         +------------+-------+----------+----------+
 *         |Netlink hdr | Align |tAniMsgHdr| msg body |
 *         +------------+-------+----------|----------+
 */

// Message Types 
#define WLAN_BTC_QUERY_STATE_REQ    0x01  // BTC  --> WLAN
#define WLAN_BTC_BT_EVENT_IND       0x02  // BTC  --> WLAN
#define WLAN_BTC_QUERY_STATE_RSP    0x03  // WLAN -->  BTC
#define WLAN_MODULE_UP_IND          0x04  // WLAN -->  BTC
#define WLAN_MODULE_DOWN_IND        0x05  // WLAN -->  BTC
#define WLAN_STA_ASSOC_DONE_IND     0x06  // WLAN -->  BTC
#define WLAN_STA_DISASSOC_DONE_IND  0x07  // WLAN -->  BTC

// Event data for WLAN_BTC_QUERY_STATE_RSP & WLAN_STA_ASSOC_DONE_IND
typedef struct
{
   unsigned char channel;  // 0 implies STA not associated to AP
} tWlanAssocData;

#define ANI_NL_MSG_BASE     0x10    /* Some arbitrary base */

typedef enum eAniNlModuleTypes {
   ANI_NL_MSG_PUMAC = ANI_NL_MSG_BASE + 0x01,// PTT Socket App
   ANI_NL_MSG_PTT   = ANI_NL_MSG_BASE + 0x07,// Quarky GUI
   WLAN_NL_MSG_BTC,
   ANI_NL_MSG_MAX  
} tAniNlModTypes, tWlanNlModTypes;

#define WLAN_NL_MSG_BASE ANI_NL_MSG_BASE
#define WLAN_NL_MSG_MAX  ANI_NL_MSG_MAX

//All Netlink messages must contain this header
typedef struct sAniHdr {
   unsigned short type;
   unsigned short length;
} tAniHdr, tAniMsgHdr;

#endif //WLAN_NLINK_COMMON_H__
