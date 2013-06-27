/*
 * Copyright (c) 2014-2010, 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
//------------------------------------------------------------------------------
// <copyright file="wmi_unified.h" company="Atheros">
//    Copyright (c) 2004-2010, 2013 Atheros Corporation.  All rights reserved.
// $ATH_LICENSE_HOSTSDK0_C$
//------------------------------------------------------------------------------
//==============================================================================
// Author(s): ="Atheros"
//==============================================================================

/**
 * @addtogroup WMIAPI
 *@{
 */

/** @file
 * This file specifies the WMI interface for the  Software Architecture.
 * 
 * It includes definitions of all the commands and events. Commands are messages
 * from the host to the target. Events and Replies are messages from the target
 * to the host.
 *
 * Ownership of correctness in regards to WMI commands
 * belongs to the host driver and the target is not required to validate
 * parameters for value, proper range, or any other checking.
 *
 * Guidelines for extending this interface are below.
 *
 * 1. Add new WMI commands ONLY within the specified range - 0x9000 - 0x9fff
 * 2. Use ONLY A_UINT32 type for defining member variables within WMI command/event
 *    structures. Do not use A_UINT8, A_UINT16, A_BOOL or enum types within these structures.
 * 3. DO NOT define bit fields within structures. Implement bit fields using masks
 *    if necessary. Do not use the programming language's bit field definition.
 * 4. Define macros for encode/decode of A_UINT8, A_UINT16 fields within the A_UINT32
 *    variables. Use these macros for set/get of these fields. Try to use this to 
 *    optimize the structure without bloating it with A_UINT32 variables for every lower
 *    sized field.
 * 5. Do not use PACK/UNPACK attributes for the structures as each member variable is
 *    already 4-byte aligned by virtue of being a A_UINT32 type.
 * 6. Comment each parameter part of the WMI command/event structure by using the
 *    2 stars at the begining of C comment instead of one star to enable HTML document
 *    generation using Doxygen. 
 *    
 */

#ifndef _WMI_UNIFIED_H_
#define _WMI_UNIFIED_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <wlan_defs.h>
#include <wmi_services.h>
#include <wal_dbg_stats.h>
#include <dbglog.h>

#define ATH_MAC_LEN             6               /**< length of MAC in bytes */
#define WMI_EVENT_STATUS_SUCCESS 0 /* Success return status to host */
#define WMI_EVENT_STATUS_FAILURE 1 /* Failure return status to host */

/*
 * These don't necessarily belong here; but as the MS/SM macros require
 * ar6000_internal.h to be included, it may not be defined as yet.
 */
#define WMI_F_MS(_v, _f)                                            \
            ( ((_v) & (_f)) >> (_f##_S) )

/*
 * This breaks the "good macro practice" of only referencing each
 * macro field once (to avoid things like field++ from causing issues.)
 */
#define WMI_F_RMW(_var, _v, _f)                                     \
            do {                                                    \
                (_var) &= ~(_f);                                    \
                (_var) |= ( ((_v) << (_f##_S)) & (_f));             \
            } while (0)

/** 2 word representation of MAC addr */
typedef struct {
    /** upper 4 bytes of  MAC address */
    A_UINT32 mac_addr31to0;
    /** lower 2 bytes of  MAC address */
    A_UINT32 mac_addr47to32;
} wmi_mac_addr;
   
/** macro to convert MAC address from WMI word format to char array */
#define WMI_MAC_ADDR_TO_CHAR_ARRAY(pwmi_mac_addr,c_macaddr) do {               \
     (c_macaddr)[0] =    ((pwmi_mac_addr)->mac_addr31to0) & 0xff;     \
     (c_macaddr)[1] =  ( ((pwmi_mac_addr)->mac_addr31to0) >> 8) & 0xff; \
     (c_macaddr)[2] =  ( ((pwmi_mac_addr)->mac_addr31to0) >> 16) & 0xff; \
     (c_macaddr)[3] =  ( ((pwmi_mac_addr)->mac_addr31to0) >> 24) & 0xff;  \
     (c_macaddr)[4] =    ((pwmi_mac_addr)->mac_addr47to32) & 0xff;        \
     (c_macaddr)[5] =  ( ((pwmi_mac_addr)->mac_addr47to32) >> 8) & 0xff; \
   } while(0) 

/** macro to convert MAC address from char array to WMI word format */
#define WMI_CHAR_ARRAY_TO_MAC_ADDR(c_macaddr,pwmi_mac_addr)  do { \
    (pwmi_mac_addr)->mac_addr31to0  =                                   \
        ( (c_macaddr)[0] | ((c_macaddr)[1] << 8)                           \
               | ((c_macaddr)[2] << 16) | ((c_macaddr)[3] << 24) );         \
    (pwmi_mac_addr)->mac_addr47to32  =                                  \
                  ( (c_macaddr)[4] | ((c_macaddr)[5] << 8));             \
   } while(0) 


/*
 * wmi command groups.
 */
typedef enum {
    /* 0 to 2 are reserved */
    WMI_GRP_START=0x3,
    WMI_GRP_SCAN=WMI_GRP_START,
    WMI_GRP_PDEV,
    WMI_GRP_VDEV,
    WMI_GRP_PEER,
    WMI_GRP_MGMT,
    WMI_GRP_BA_NEG,
    WMI_GRP_STA_PS,
    WMI_GRP_DFS,
    WMI_GRP_ROAM,
    WMI_GRP_OFL_SCAN,
    WMI_GRP_P2P,
    WMI_GRP_AP_PS,
    WMI_GRP_RATE_CTRL,
    WMI_GRP_PROFILE,
    WMI_GRP_SUSPEND,
    WMI_GRP_BCN_FILTER,
    WMI_GRP_WOW,
    WMI_GRP_RTT,
    WMI_GRP_SPECTRAL,
    WMI_GRP_STATS,
    WMI_GRP_ARP_NS_OFL,
    WMI_GRP_NLO_OFL,
    WMI_GRP_GTK_OFL,
    WMI_GRP_CSA_OFL,
    WMI_GRP_CHATTER,
    WMI_GRP_TID_ADDBA,
    WMI_GRP_MISC,
} WMI_GRP_ID;

#define WMI_CMD_GRP_START_ID(grp_id) (((grp_id) << 12) | 0x1)
#define WMI_EVT_GRP_START_ID(grp_id) (((grp_id) << 12) | 0x1)

/**
 * Command IDs and commange events.
 */
typedef enum {
    /** initialize the wlan sub system */
    WMI_INIT_CMDID=0x1,    

    /* Scan specific commands */

    /** start scan request to FW  */
    WMI_START_SCAN_CMDID = WMI_CMD_GRP_START_ID(WMI_GRP_SCAN) , 
    /** stop scan request to FW  */
    WMI_STOP_SCAN_CMDID,
    /** full list of channels as defined by the regulatory that will be used by scanner   */
    WMI_SCAN_CHAN_LIST_CMDID,
    /** overwrite default priority table in scan scheduler   */
    WMI_SCAN_SCH_PRIO_TBL_CMDID,

    /* PDEV(physical device) specific commands */
    /** set regulatorty ctl id used by FW to determine the exact ctl power limits */
    WMI_PDEV_SET_REGDOMAIN_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_PDEV),
    /** set channel. mainly used for supporting monitor mode */
    WMI_PDEV_SET_CHANNEL_CMDID,
    /** set pdev specific parameters */
    WMI_PDEV_SET_PARAM_CMDID,
    /** enable packet log */
    WMI_PDEV_PKTLOG_ENABLE_CMDID,
    /** disable packet log*/
    WMI_PDEV_PKTLOG_DISABLE_CMDID,
    /** set wmm parameters */
    WMI_PDEV_SET_WMM_PARAMS_CMDID,
    /** set HT cap ie that needs to be carried probe requests HT/VHT channels */
    WMI_PDEV_SET_HT_CAP_IE_CMDID,
    /** set VHT cap ie that needs to be carried on probe requests on VHT channels */
    WMI_PDEV_SET_VHT_CAP_IE_CMDID,

    /** Command to send the DSCP-to-TID map to the target */
    WMI_PDEV_SET_DSCP_TID_MAP_CMDID,
    /** set quiet ie parameters. primarily used in AP mode */
    WMI_PDEV_SET_QUIET_MODE_CMDID,
    /** Enable/Disable Green AP Power Save  */
    WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID,
    /** get TPC config for the current operating channel */
    WMI_PDEV_GET_TPC_CONFIG_CMDID,

    /** set the base MAC address for the physical device before a VDEV is created.
     *  For firmware that doesn’t support this feature and this command, the pdev
     *  MAC address will not be changed. */
    WMI_PDEV_SET_BASE_MACADDR_CMDID,

    /* VDEV(virtual device) specific commands */
    /** vdev create */
    WMI_VDEV_CREATE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_VDEV),
    /** vdev delete */
    WMI_VDEV_DELETE_CMDID,
    /** vdev start request */
    WMI_VDEV_START_REQUEST_CMDID,
    /** vdev restart request (RX only, NO TX, used for CAC period)*/
    WMI_VDEV_RESTART_REQUEST_CMDID,
    /** vdev up request */
    WMI_VDEV_UP_CMDID,
    /** vdev stop request */
    WMI_VDEV_STOP_CMDID,
    /** vdev down request */
    WMI_VDEV_DOWN_CMDID,
    /** vdev stanby response. sent by host in respose to standby request from FW
     *  used part of message exchange with FW to move AP vdev to a new channel 
     *  along with STA vdev when the STA vdev moves to a new channel */
    WMI_VDEV_STANDBY_RESPONSE_CMDID,
    /** vdev resume response. sent by host in respose to resume request from FW 
     *  used part of message exchange with FW to move AP vdev to a new channel 
     *  along with STA vdev when the STA vdev moves to a new channel */
    WMI_VDEV_RESUME_RESPONSE_CMDID,
    /* set a vdev param */
    WMI_VDEV_SET_PARAM_CMDID,
    /* set a key (used for setting per peer unicast and per vdev multicast) */
    WMI_VDEV_INSTALL_KEY_CMDID,

    /* peer specific commands */

    /** create a peer */
    WMI_PEER_CREATE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_PEER),
    /** delete a peer */
    WMI_PEER_DELETE_CMDID,
    /** flush specific  tid queues of a peer */
    WMI_PEER_FLUSH_TIDS_CMDID,
    /** set a parameter of a peer */
    WMI_PEER_SET_PARAM_CMDID,
    /** set peer to associated state. will cary all parameters determined during assocication time */
    WMI_PEER_ASSOC_CMDID,
    /**add a wds  (4 address ) entry. used only for testing WDS feature on AP products */
    WMI_PEER_ADD_WDS_ENTRY_CMDID,
    /**remove wds  (4 address ) entry. used only for testing WDS feature on AP products */
    WMI_PEER_REMOVE_WDS_ENTRY_CMDID,
    /** set up mcast group infor for multicast to unicast conversion */
    WMI_PEER_MCAST_GROUP_CMDID,

    /* beacon/management specific commands */

    /** transmit beacon by reference . used for transmitting beacon on low latency interface like pcie */
    WMI_BCN_TX_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_MGMT),
    /** transmit beacon by value */
    WMI_PDEV_SEND_BCN_CMDID,
    /** set the beacon template. used in beacon offload mode to setup the 
     *  the common beacon template with the FW to be used by FW to generate beacons */
    WMI_BCN_TMPL_CMDID,
    /** set beacon filter with FW */ 
    WMI_BCN_FILTER_RX_CMDID,
    /* enable/disable filtering of probe requests in the firmware */
    WMI_PRB_REQ_FILTER_RX_CMDID,
    /** transmit management frame by value. will be deprecated */ 
    WMI_MGMT_TX_CMDID,
    /** set the probe response template. used in beacon offload mode to setup the 
     *  the common probe response template with the FW to be used by FW to generate 
     *  probe responses */
    WMI_PRB_TMPL_CMDID,

    /** commands to directly control ba negotiation directly from host. only used in test mode */

    /** turn off FW Auto addba mode and let host control addba */
    WMI_ADDBA_CLEAR_RESP_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_BA_NEG),
    /** send add ba request */
    WMI_ADDBA_SEND_CMDID,
    WMI_ADDBA_STATUS_CMDID,
    /** send del ba */
    WMI_DELBA_SEND_CMDID,
    /** set add ba response will be used by FW to generate addba response*/
    WMI_ADDBA_SET_RESP_CMDID,
    /** send single VHT MPDU with AMSDU */
    WMI_SEND_SINGLEAMSDU_CMDID,

    /** Station power save specific config */
    /** enable/disable station powersave */
    WMI_STA_POWERSAVE_MODE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_STA_PS),
    /** set station power save specific parameter */
    WMI_STA_POWERSAVE_PARAM_CMDID,
    /** set station mimo powersave mode */
    WMI_STA_MIMO_PS_MODE_CMDID,
    
    
    /** DFS-specific commands */
    /** enable DFS (radar detection)*/
    WMI_PDEV_DFS_ENABLE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_DFS),
    /** disable DFS (radar detection)*/
    WMI_PDEV_DFS_DISABLE_CMDID,


    /* Roaming specific  commands */
    /** set roam scan mode */
    WMI_ROAM_SCAN_MODE=WMI_CMD_GRP_START_ID(WMI_GRP_ROAM),
    /** set roam scan rssi threshold below which roam scan is enabled  */
    WMI_ROAM_SCAN_RSSI_THRESHOLD,
    /** set roam scan period for periodic roam scan mode  */
    WMI_ROAM_SCAN_PERIOD,
    /** set roam scan trigger rssi change threshold   */
    WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD,
    /** set roam AP profile   */
    WMI_ROAM_AP_PROFILE,

    /** offload scan specific commands */
    /** set offload scan AP profile   */
    WMI_OFL_SCAN_ADD_AP_PROFILE=WMI_CMD_GRP_START_ID(WMI_GRP_OFL_SCAN),
    /** remove offload scan AP profile   */
    WMI_OFL_SCAN_REMOVE_AP_PROFILE,
    /** set offload scan period   */
    WMI_OFL_SCAN_PERIOD,

    /* P2P specific commands */
    /**set P2P device info. FW will used by FW to create P2P IE to be carried in probe response
     * generated during p2p listen and for p2p discoverability  */
    WMI_P2P_DEV_SET_DEVICE_INFO=WMI_CMD_GRP_START_ID(WMI_GRP_P2P),
    /** enable/disable p2p discoverability on STA/AP VDEVs  */
    WMI_P2P_DEV_SET_DISCOVERABILITY,
    /** set p2p ie to be carried in beacons generated by FW for GO  */
    WMI_P2P_GO_SET_BEACON_IE,
    /** set p2p ie to be carried in probe response frames generated by FW for GO  */
    WMI_P2P_GO_SET_PROBE_RESP_IE,
    /** set the vendor specific p2p ie data. FW will use this to parse the P2P NoA
     *  attribute in the beacons/probe responses received.
     */
    WMI_P2P_SET_VENDOR_IE_DATA_CMDID,


    /** AP power save specific config */
    /** set AP power save specific param */
    WMI_AP_PS_PEER_PARAM_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_AP_PS),
    /** set AP UAPSD coex pecific param */
    WMI_AP_PS_PEER_UAPSD_COEX_CMDID,


    /** Rate-control specific commands */
    WMI_PEER_RATE_RETRY_SCHED_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_RATE_CTRL),

    /** WLAN Profiling commands. */
    WMI_WLAN_PROFILE_TRIGGER_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_PROFILE),
    WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
    WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
    WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
    WMI_WLAN_PROFILE_LIST_PROFILE_ID_CMDID,

    /** Suspend resume command Ids */
    WMI_PDEV_SUSPEND_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_SUSPEND),
    WMI_PDEV_RESUME_CMDID,

    /* Beacon filter commands */
    /** add a beacon filter */
    WMI_ADD_BCN_FILTER_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_BCN_FILTER),
    /** remove a  beacon filter */
    WMI_RMV_BCN_FILTER_CMDID,

    /* WOW Specific WMI commands*/
    /** add pattern for awake */  
    WMI_WOW_ADD_WAKE_PATTERN_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_WOW),
    /** deleta a wake pattern */  
    WMI_WOW_DEL_WAKE_PATTERN_CMDID,
    /** enable/deisable wake event  */  
    WMI_WOW_ENABLE_DISABLE_WAKE_EVENT_CMDID,
    /** enable WOW  */  
    WMI_WOW_ENABLE_CMDID,
    /** host woke up from sleep event to FW. Generated in response to WOW Hardware event */  
    WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID,

    /* RTT measurement related cmd */
    /** reques to make an RTT measurement */ 
    WMI_RTT_MEASREQ_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_RTT),     
    /** reques to report a tsf measurement */ 
    WMI_RTT_TSF_CMDID,         

    /** spectral scan command */
    /** configure spectral scan */
    WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_SPECTRAL),
    /** enable/disable spectral scan and trigger */
    WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID,

    /* F/W stats */
    /** one time request for stats */
    WMI_REQUEST_STATS_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_STATS),

    /** ARP OFFLOAD REQUEST*/
    WMI_SET_ARP_NS_OFFLOAD_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_ARP_NS_OFL),

    /** NS offload confid*/
    WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_NLO_OFL),

	/* GTK offload Specific WMI commands*/
	WMI_GTK_OFFLOAD_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_GTK_OFL),

	/* CSA offload Specific WMI commands*/
    /** csa offload enable */   
    WMI_CSA_OFFLOAD_ENABLE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_CSA_OFL),
    /** chan switch command */   
    WMI_CSA_OFFLOAD_CHANSWITCH_CMDID,

    /* Chatter commands*/
    /* Change chatter mode of operation */
    WMI_CHATTER_SET_MODE_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_CHATTER),

    /**addba specific commands */
    /** start the aggregation on this TID */
    WMI_PEER_TID_ADDBA_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_TID_ADDBA),
    /** stop the aggregation on this TID */
    WMI_PEER_TID_DELBA_CMDID,


    /* misc command group */
    /** echo command mainly used for testing */
    WMI_ECHO_CMDID=WMI_CMD_GRP_START_ID(WMI_GRP_MISC),
    /** set debug log config */
    WMI_DBGLOG_CFG_CMDID,
    /* QVIT specific command id */
    WMI_PDEV_QVIT_CMDID,
    /* Factory Testing Mode request command
     * used for integrated chipsets */
    WMI_PDEV_FTM_INTG_CMDID,
    /* set and get keepalive parameters command */
    WMI_VDEV_SET_KEEPALIVE_CMDID,
    WMI_VDEV_GET_KEEPALIVE_CMDID,
    /** UTF specific WMI commands 
     * set fixed value for UTF WMI command so 
     * further addition of other WMI commands
     * does not affect the communication between
     * ART2 and UTF
     */
    WMI_PDEV_UTF_CMDID
} WMI_CMD_ID;

typedef enum {
    /** WMI service is ready; after this event WMI messages can be sent/received  */
    WMI_SERVICE_READY_EVENTID=0x1, 
    /** WMI is ready; after this event the wlan subsystem is initialized and can process commands. */
    WMI_READY_EVENTID,            
    
    /** Scan specific events */
    WMI_SCAN_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_SCAN) , 

    /* PDEV specific events */
    /** TPC config for the current operating channel */
    WMI_PDEV_TPC_CONFIG_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_PDEV) ,
    /** Channel stats event    */
    WMI_CHAN_INFO_EVENTID,

    /** PHY Error specific WMI event */
    WMI_PHYERR_EVENTID,

    /* VDEV specific events */
    /** VDEV started event in response to VDEV_START request */
    WMI_VDEV_START_RESP_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_VDEV),
    /** vdev stanby request.this is  
     *  used part of message exchange with FW to move AP vdev to a new channel 
     *  along with STA vdev when the STA vdev moves to a new channel */
    WMI_VDEV_STANDBY_REQ_EVENTID,
    /** vdev resume request.this is  
     *  used part of message exchange with FW to move AP vdev to a new channel 
     *  along with STA vdev when the STA vdev moves to a new channel */
    WMI_VDEV_RESUME_REQ_EVENTID,
    /** vdev stopped event , generated in response to VDEV_STOP request */  
    WMI_VDEV_STOPPED_EVENTID,
    /* Indicate the set key (used for setting per 
     * peer unicast and per vdev multicast) 
     * operation has completed */
    WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID,

    /* peer  specific events */
    /** FW reauet to kick out the station for reasons like inactivity,lack of response ..etc */
    WMI_PEER_STA_KICKOUT_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_PEER),

    /* beacon/mgmt specific events */
    /** RX management frame. the entire frame is carried along with the event.  */
    WMI_MGMT_RX_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_MGMT),
    /** software beacon alert event to Host requesting host to Queue a beacon for transmission
        use only in host beacon mode */
    WMI_HOST_SWBA_EVENTID,
    /** beacon tbtt offset event indicating the tsf offset of the tbtt from the theritical value.
        tbtt offset is normally 0 and will be non zero if there are multiple VDEVs operating in 
        staggered beacon transmission mode */ 
    WMI_TBTTOFFSET_UPDATE_EVENTID,

    /*ADDBA Related WMI Events*/
    /** Indication the completion of the prior
       WMI_PEER_TID_DELBA_CMDID(initiator) */
    WMI_TX_DELBA_COMPLETE_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_BA_NEG),
    /** Indication the completion of the prior
       *WMI_PEER_TID_ADDBA_CMDID(initiator) */
    WMI_TX_ADDBA_COMPLETE_EVENTID,

    /** Roam event to trigger roaming on host */
    WMI_ROAM_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_ROAM),

    /** matching AP found from list of profiles */
    WMI_PROFILE_MATCH,
    
    /** WOW wake up host event.generated in response to WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID. 
        will cary wake reason */ 
    WMI_WOW_WAKEUP_HOST_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_WOW),

    /*RTT related event ID*/
    /** RTT measurement report */ 
    WMI_RTT_MEASUREMENT_REPORT_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_RTT),     
    /** TSF measurement report */ 
    WMI_TSF_MEASUREMENT_REPORT_EVENTID, 
    /** RTT error report */ 
    WMI_RTT_ERROR_REPORT_EVENTID,      



    
    /** GTK offload stautus event requested by host */
	WMI_GTK_OFFLOAD_STATUS_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_GTK_OFL),  
		
	/** GTK offload failed to rekey event */
	WMI_GTK_REKEY_FAIL_EVENTID,
    /* CSA IE received event */
    WMI_CSA_HANDLING_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_CSA_OFL),


    /** echo event in response to echo command */
    WMI_ECHO_EVENTID = WMI_EVT_GRP_START_ID(WMI_GRP_MISC),
    /** event carries buffered debug messages  */
    WMI_DEBUG_MESG_EVENTID, 
    /** FW stats(periodic or on shot)  */
    WMI_UPDATE_STATS_EVENTID,
    /** debug print message used for tracing FW code while debugging  */
    WMI_DEBUG_PRINT_EVENTID, 
    /** DCS wlan or non-wlan interference event
     */
    WMI_DCS_INTERFERENCE_EVENTID,
    /** VI spoecific event  */
    WMI_PDEV_QVIT_EVENTID,
    /** FW code profile data in response to profile request  */
    WMI_WLAN_PROFILE_DATA_EVENTID,
    /* Factory Testing Mode request event
     * used for integrated chipsets */
    WMI_PDEV_FTM_INTG_EVENTID,
    /* avoid list of frequencies .
     */
    WMI_WLAN_FREQ_AVOID_EVENTID,
    /* Indicate the keepalive parameters */
    WMI_VDEV_GET_KEEPALIVE_EVENTID,
    /** UTF specific WMI event 
     * set fixed value for UTF WMI EVT ID so 
     * further addition of other WMI EVT IDs
     * does not affect the communication between
     * ART2 and UTF
     */
    WMI_PDEV_UTF_EVENTID,
} WMI_EVT_ID;


#define WMI_CHAN_LIST_TAG 0x1
#define WMI_SSID_LIST_TAG 0x2
#define WMI_BSSID_LIST_TAG 0x3
#define WMI_IE_TAG 0x4

typedef struct {
    /** primary 20 MHz channel frequency in mhz */ 
    A_UINT32 mhz;
    /** Center frequency 1 in MHz*/
    A_UINT32 band_center_freq1;
    /** Center frequency 2 in MHz - valid only for 11acvht 80plus80 mode*/
    A_UINT32 band_center_freq2;
    /** channel info described below */ 
    A_UINT32 info; 
    /** contains min power, max power, reg power and reg class id.  */ 
    A_UINT32 reg_info_1;
    /** contains antennamax */
    A_UINT32 reg_info_2;
} wmi_channel;

typedef enum{
WMI_CHANNEL_CHANGE_CAUSE_NONE = 0,
WMI_CHANNEL_CHANGE_CAUSE_CSA,
}wmi_channel_change_cause;

/** channel info consists of 6 bits of channel mode */

#define WMI_SET_CHANNEL_MODE(pwmi_channel,val) do { \
     (pwmi_channel)->info &= 0xffffffc0;            \
     (pwmi_channel)->info |= (val);                 \
     } while(0)

#define WMI_GET_CHANNEL_MODE(pwmi_channel) ((pwmi_channel)->info & 0x0000003f ) 

#define WMI_CHAN_FLAG_HT40_PLUS   6
#define WMI_CHAN_FLAG_PASSIVE     7
#define WMI_CHAN_ADHOC_ALLOWED    8
#define WMI_CHAN_AP_DISABLED      9
#define WMI_CHAN_FLAG_DFS         10
#define WMI_CHAN_FLAG_ALLOW_HT    11  /* HT is allowed on this channel */
#define WMI_CHAN_FLAG_ALLOW_VHT   12  /* VHT is allowed on this channel */
#define WMI_CHANNEL_CHANGE_CAUSE_CSA 13 /*Indicate reason for channel switch */

#define WMI_SET_CHANNEL_FLAG(pwmi_channel,flag) do { \
        (pwmi_channel)->info |=  (1 << flag);      \
     } while(0)

#define WMI_GET_CHANNEL_FLAG(pwmi_channel,flag)   \
        (((pwmi_channel)->info & (1 << flag)) >> flag)

#define WMI_SET_CHANNEL_MIN_POWER(pwmi_channel,val) do { \
     (pwmi_channel)->reg_info_1 &= 0xffffff00;            \
     (pwmi_channel)->reg_info_1 |= (val);                 \
     } while(0)
#define WMI_GET_CHANNEL_MIN_POWER(pwmi_channel) ((pwmi_channel)->reg_info_1 & 0xff ) 

#define WMI_SET_CHANNEL_MAX_POWER(pwmi_channel,val) do { \
     (pwmi_channel)->reg_info_1 &= 0xffff00ff;            \
     (pwmi_channel)->reg_info_1 |= ((val) << 8);          \
     } while(0)
#define WMI_GET_CHANNEL_MAX_POWER(pwmi_channel) ( (((pwmi_channel)->reg_info_1) >> 8) & 0xff ) 

#define WMI_SET_CHANNEL_REG_POWER(pwmi_channel,val) do { \
     (pwmi_channel)->reg_info_1 &= 0xff00ffff;            \
     (pwmi_channel)->reg_info_1 |= ((val) << 16);         \
     } while(0)
#define WMI_GET_CHANNEL_REG_POWER(pwmi_channel) ( (((pwmi_channel)->reg_info_1) >> 16) & 0xff ) 
#define WMI_SET_CHANNEL_REG_CLASSID(pwmi_channel,val) do { \
     (pwmi_channel)->reg_info_1 &= 0x00ffffff;             \
     (pwmi_channel)->reg_info_1 |= ((val) << 24);          \
     } while(0)
#define WMI_GET_CHANNEL_REG_CLASSID(pwmi_channel) ( (((pwmi_channel)->reg_info_1) >> 24) & 0xff ) 

#define WMI_SET_CHANNEL_ANTENNA_MAX(pwmi_channel,val) do { \
     (pwmi_channel)->reg_info_2 &= 0xffffff00;            \
     (pwmi_channel)->reg_info_2 |= (val);                 \
     } while(0)
#define WMI_GET_CHANNEL_ANTENNA_MAX(pwmi_channel) ((pwmi_channel)->reg_info_2 & 0xff ) 


/** HT Capabilities*/
#define WMI_HT_CAP_ENABLED                0x0001   /* HT Enabled/ disabled */
#define WMI_HT_CAP_HT20_SGI               0x0002   /* Short Guard Interval with HT20 */
#define WMI_HT_CAP_DYNAMIC_SMPS           0x0004   /* Dynamic MIMO powersave */
#define WMI_HT_CAP_TX_STBC                0x0008   /* B3 TX STBC */
#define WMI_HT_CAP_TX_STBC_MASK_SHIFT     3
#define WMI_HT_CAP_RX_STBC                0x0030   /* B4-B5 RX STBC */
#define WMI_HT_CAP_RX_STBC_MASK_SHIFT     4
#define WMI_HT_CAP_LDPC                   0x0040   /* LDPC supported */
#define WMI_HT_CAP_L_SIG_TXOP_PROT        0x0080   /* L-SIG TXOP Protection */
#define WMI_HT_CAP_MPDU_DENSITY           0x0700   /* MPDU Density */
#define WMI_HT_CAP_MPDU_DENSITY_MASK_SHIFT 8
#define WMI_HT_CAP_HT40_SGI               0x0800

/* These macros should be used when we wish to advertise STBC support for
 * only 1SS or 2SS or 3SS. */
#define WMI_HT_CAP_RX_STBC_1SS            0x0010   /* B4-B5 RX STBC */
#define WMI_HT_CAP_RX_STBC_2SS            0x0020   /* B4-B5 RX STBC */
#define WMI_HT_CAP_RX_STBC_3SS            0x0030   /* B4-B5 RX STBC */


#define WMI_HT_CAP_DEFAULT_ALL (WMI_HT_CAP_ENABLED       | \
                                WMI_HT_CAP_HT20_SGI      | \
                                WMI_HT_CAP_HT40_SGI      | \
                                WMI_HT_CAP_TX_STBC       | \
                                WMI_HT_CAP_RX_STBC       | \
                                WMI_HT_CAP_LDPC)

/* WMI_VHT_CAP_* these maps to ieee 802.11ac vht capability information
   field. The fields not defined here are not supported, or reserved.
   Do not change these masks and if you have to add new one follow the
   bitmask as specified by 802.11ac draft.
*/

#define WMI_VHT_CAP_MAX_MPDU_LEN_MASK            0x00000003
#define WMI_VHT_CAP_RX_LDPC                      0x00000010
#define WMI_VHT_CAP_SGI_80MHZ                    0x00000020
#define WMI_VHT_CAP_TX_STBC                      0x00000080
#define WMI_VHT_CAP_RX_STBC_MASK                 0x00000300
#define WMI_VHT_CAP_RX_STBC_MASK_SHIFT           8
#define WMI_VHT_CAP_MAX_AMPDU_LEN_EXP            0x03800000
#define WMI_VHT_CAP_MAX_AMPDU_LEN_EXP_SHIT       23
#define WMI_VHT_CAP_RX_FIXED_ANT                 0x10000000
#define WMI_VHT_CAP_TX_FIXED_ANT                 0x20000000

#define WMI_VHT_CAP_MAX_MPDU_LEN_11454           0x00000002

/* These macros should be used when we wish to advertise STBC support for
 * only 1SS or 2SS or 3SS. */
#define WMI_VHT_CAP_RX_STBC_1SS            0x00000100
#define WMI_VHT_CAP_RX_STBC_2SS            0x00000200
#define WMI_vHT_CAP_RX_STBC_3SS            0x00000300

#define WMI_VHT_CAP_DEFAULT_ALL (WMI_VHT_CAP_MAX_MPDU_LEN_11454  |      \
                                 WMI_VHT_CAP_SGI_80MHZ           |      \
                                 WMI_VHT_CAP_TX_STBC             |      \
                                 WMI_VHT_CAP_RX_STBC_MASK        |      \
                                 WMI_VHT_CAP_RX_LDPC             |      \
                                 WMI_VHT_CAP_MAX_AMPDU_LEN_EXP   |      \
                                 WMI_VHT_CAP_RX_FIXED_ANT        |      \
                                 WMI_VHT_CAP_TX_FIXED_ANT)

/* Interested readers refer to Rx/Tx MCS Map definition as defined in
   802.11ac 
*/
#define WMI_VHT_MAX_MCS_4_SS_MASK(r,ss)      ((3 & (r)) << (((ss) - 1) << 1))
#define WMI_VHT_MAX_SUPP_RATE_MASK           0x1fff0000
#define WMI_VHT_MAX_SUPP_RATE_MASK_SHIFT     16

/* WMI_SYS_CAPS_* refer to the capabilities that system support
*/
#define WMI_SYS_CAP_ENABLE                       0x00000001
#define WMI_SYS_CAP_TXPOWER                      0x00000002


#define WMI_NUM_UNITS_IS_NUM_VDEVS   0x1
#define WMI_NUM_UNITS_IS_NUM_PEERS   0x2
typedef struct {
    /** ID of the request */
    A_UINT32    req_id; 
    /** size of the  of each unit */
    A_UINT32    unit_size; 
    /** 
     * flags to  indicate that 
     * the number units is dependent 
     * on number of resources(num vdevs num peers .. etc) 
     */
    A_UINT32    num_unit_info; 
    /*
     * actual number of units to allocate . if flags in the num_unit_info 
     * indicate that number of units is tied to number of a particular
     * resource to allocate then  num_units filed is set to 0 and host 
     * will derive the number units from number of the resources it is 
     * requesting.
     */
    A_UINT32    num_units; 
} wmi_mem_req;

/*
* maximum number of memroy requests allowed from FW.
*/
#define WMI_MAX_MEM_REQS 16
/**
 * The following struct holds optional payload for 
 * wmi_service_ready_event,e.g., 11ac pass some of the 
 * device capability to the host.
*/
typedef struct {
    A_UINT32    sw_version; /* first dword of version info */
    A_UINT32    sw_version_1; /* second dword of version info */
    A_UINT32    abi_version;
    A_UINT32    phy_capability;  /* WMI_PHY_CAPABILITY */
	A_UINT32    max_frag_entry;  /* Maximum number of frag table entries that SW will populate less 1 */
    A_UINT32    wmi_service_bitmap[WMI_SERVICE_BM_SIZE];   
    A_UINT32    num_rf_chains;
    /* The following field is only valid for service type WMI_SERVICE_11AC */
    A_UINT32    ht_cap_info; /* WMI HT Capability */
    A_UINT32    vht_cap_info; /* VHT capability info field of 802.11ac */
    A_UINT32    vht_supp_mcs; /* VHT Supported MCS Set field Rx/Tx same */
    A_UINT32    hw_min_tx_power;   
    A_UINT32    hw_max_tx_power;   
    HAL_REG_CAPABILITIES hal_reg_capabilities;
    A_UINT32    sys_cap_info;    
    A_UINT32    min_pkt_size_enable; /* Enterprise mode short pkt enable */    
    /** Max beacon and Probe Response IE offload size (includes
     *  optional P2P IEs) */
    A_UINT32    max_bcn_ie_size;
    /*
     * request to host to allocate a chuck of memory and pss it down to FW via WM_INIT. 
     * FW uses this as FW extesnsion memory for saving its data structures. Only valid
     * for low latency interfaces like PCIE where FW can access this memory directly (or)
     * by DMA.
     */
    A_UINT32    num_mem_reqs; 
    wlan_host_mem_req  mem_reqs[1];
} wmi_service_ready_event;

/** status consists of  upper 16 bits fo A_STATUS status and lower 16 bits of module ID that retuned status */
#define WLAN_INIT_STATUS_SUCCESS   0x0
#define WLAN_GET_INIT_STATUS_REASON(status)    ((status) & 0xffff) 
#define WLAN_GET_INIT_STATUS_MODULE_ID(status) (((status) >> 16) & 0xffff) 

typedef A_UINT32 WLAN_INIT_STATUS;

typedef struct {
    A_UINT32    sw_version;
    A_UINT32    abi_version;
    wmi_mac_addr mac_addr;
    A_UINT32    status;
} wmi_ready_event;

typedef struct {
/**
 * @brief num_vdev - number of virtual devices (VAPs) to support
 */
    A_UINT32 num_vdevs;
/**
 * @brief num_peers - number of peer nodes to support
 */
    A_UINT32 num_peers;
/*
 * @brief In offload mode target supports features like WOW, chatter and other
 * protocol offloads. In order to support them some functionalities like
 * reorder buffering, PN checking need to be done in target. This determines
 * maximum number of peers suported by target in offload mode
 */
    A_UINT32 num_offload_peers;
/* @brief Number of reorder buffers available for doing target based reorder
 * Rx reorder buffering
 */
    A_UINT32 num_offload_reorder_buffs;
/**
 * @brief num_peer_keys - number of keys per peer
 */
    A_UINT32 num_peer_keys;
/**
 * @brief num_peer_tids - number of TIDs to provide storage for per peer.
 */
    A_UINT32 num_tids;
/**
 * @brief ast_skid_limit - max skid for resolving hash collisions
 * @details
 *     The address search table is sparse, so that if two MAC addresses
 *     result in the same hash value, the second of these conflicting
 *     entries can slide to the next index in the address search table,
 *     and use it, if it is unoccupied.  This ast_skid_limit parameter
 *     specifies the upper bound on how many subsequent indices to search
 *     over to find an unoccupied space.
 */
    A_UINT32 ast_skid_limit;
/**
 * @brief tx_chain_mask - the nominal chain mask for transmit
 * @details
 *     The chain mask may be modified dynamically, e.g. to operate AP tx with
 *     a reduced number of chains if no clients are associated.
 *     This configuration parameter specifies the nominal chain-mask that
 *     should be used when not operating with a reduced set of tx chains.
 */
    A_UINT32 tx_chain_mask;
/**
 * @brief rx_chain_mask - the nominal chain mask for receive
 * @details
 *     The chain mask may be modified dynamically, e.g. for a client to use
 *     a reduced number of chains for receive if the traffic to the client
 *     is low enough that it doesn't require downlink MIMO or antenna
 *     diversity.
 *     This configuration parameter specifies the nominal chain-mask that
 *     should be used when not operating with a reduced set of rx chains.
 */
    A_UINT32 rx_chain_mask;
/**
 * @brief rx_timeout_pri - what rx reorder timeout (ms) to use for the AC
 * @details
 *     Each WMM access class (voice, video, best-effort, background) will
 *     have its own timeout value to dictate how long to wait for missing
 *     rx MPDUs to arrive before flushing subsequent MPDUs that have already
 *     been received.
 *     This parameter specifies the timeout in milliseconds for each class .
 */
    A_UINT32 rx_timeout_pri[4];
/**
 * @brief rx_decap mode - what mode the rx should decap packets to
 * @details
 *     MAC can decap to RAW (no decap), native wifi or Ethernet types
 *     THis setting also determines the default TX behavior, however TX  
 *     behavior can be modified on a per VAP basis during VAP init
 */
    A_UINT32 rx_decap_mode;
 /**
  * @brief  scan_max_pending_req - what is the maximum scan requests than can be queued
  */
    A_UINT32 scan_max_pending_req;

    /**
     * @brief maximum VDEV that could use BMISS offload
     */
    A_UINT32 bmiss_offload_max_vdev;

    /**
     * @brief maximum VDEV that could use offload roaming
     */
    A_UINT32 roam_offload_max_vdev;

    /**
     * @brief maximum AP profiles that would push to offload roaming
     */
    A_UINT32 roam_offload_max_ap_profiles;

/**
 * @brief num_mcast_groups - how many groups to use for mcast->ucast conversion
 * @details
 *     The target's WAL maintains a table to hold information regarding which
 *     peers belong to a given multicast group, so that if multicast->unicast
 *     conversion is enabled, the target can convert multicast tx frames to a
 *     series of unicast tx frames, to each peer within the multicast group.
 *     This num_mcast_groups configuration parameter tells the target how
 *     many multicast groups to provide storage for within its multicast
 *     group membership table.
 */
    A_UINT32 num_mcast_groups;

/**
 * @brief num_mcast_table_elems - size to alloc for the mcast membership table
 * @details
 *     This num_mcast_table_elems configuration parameter tells the target
 *     how many peer elements it needs to provide storage for in its
 *     multicast group membership table.
 *     These multicast group membership table elements are shared by the
 *     multicast groups stored within the table.
 */
    A_UINT32 num_mcast_table_elems;

/**
 * @brief mcast2ucast_mode - whether/how to do multicast->unicast conversion
 * @details
 *     This configuration parameter specifies whether the target should
 *     perform multicast --> unicast conversion on transmit, and if so,
 *     what to do if it finds no entries in its multicast group membership
 *     table for the multicast IP address in the tx frame.
 *     Configuration value:
 *     0 -> Do not perform multicast to unicast conversion.
 *     1 -> Convert multicast frames to unicast, if the IP multicast address
 *          from the tx frame is found in the multicast group membership
 *          table.  If the IP multicast address is not found, drop the frame.
 *     2 -> Convert multicast frames to unicast, if the IP multicast address
 *          from the tx frame is found in the multicast group membership
 *          table.  If the IP multicast address is not found, transmit the
 *          frame as multicast.
 */
    A_UINT32 mcast2ucast_mode;


 /**
  * @brief tx_dbg_log_size - how much memory to allocate for a tx PPDU dbg log
  * @details
  *     This parameter controls how much memory the target will allocate to
  *     store a log of tx PPDU meta-information (how large the PPDU was,
  *     when it was sent, whether it was successful, etc.)
  */
    A_UINT32 tx_dbg_log_size;

 /**
  * @brief num_wds_entries - how many AST entries to be allocated for WDS 
  */
    A_UINT32 num_wds_entries;

 /**
  * @brief dma_burst_size - MAC DMA burst size, e.g., on Peregrine on PCI
  * this limit can be 0 -default, 1 256B
  */
    A_UINT32 dma_burst_size;

  /**
   * @brief mac_aggr_delim - Fixed delimiters to be inserted after every MPDU
   * to account for interface latency to avoid underrun.
   */
    A_UINT32 mac_aggr_delim;
    /**
     * @brief rx_skip_defrag_timeout_dup_detection_check 
     * @details
     *  determine whether target is responsible for detecting duplicate
     *  non-aggregate MPDU and timing out stale fragments.
     *
     *  A-MPDU reordering is always performed on the target.
     *
     *  0: target responsible for frag timeout and dup checking
     *  1: host responsible for frag timeout and dup checking
     */
    A_UINT32 rx_skip_defrag_timeout_dup_detection_check;
    
    /**
     * @brief vow_config - Configuration for VoW : No of Video Nodes to be supported
     * and Max no of descriptors for each Video link (node).
     */
    A_UINT32 vow_config;
	 
    /**
     * @brief maximum VDEV that could use GTK offload
     */
	A_UINT32 gtk_offload_max_vdev;

    /**
     * @brief num_msdu_desc - Number of msdu descriptors target should use
     */
    A_UINT32 num_msdu_desc; /* Number of msdu desc */
 /**
  * @brief max_frag_entry - Max. number of Tx fragments per MSDU 
  * @details
  *     This parameter controls the max number of Tx fragments per MSDU.
  *     This is sent by the target as part of the WMI_SERVICE_READY event
  *     and is overriden by the OS shim as required.
  */
    A_UINT32 max_frag_entries;
 
} wmi_resource_config;


typedef struct {
    wmi_resource_config   resource_config;
    A_UINT32              num_host_mem_chunks;
    /* 
     * variable number of host memory chunks.
     * This should be the last element in the structure
     */
    wlan_host_memory_chunk host_mem_chunks[1];
} wmi_init_cmd;
/**
 * TLV for channel list 
 */
typedef struct {
    /** WMI_CHAN_LIST_TAG */
    A_UINT32     tag;
    /** # if channels to scan */
    A_UINT32 num_chan;
    /** channels in Mhz */
    A_UINT32 channel_list[1];
} wmi_chan_list;


/**
 * TLV for bssid list 
 */
typedef struct {
    /** WMI_BSSID_LIST_TAG */
    A_UINT32     tag;  
    /** number of bssids   */
    A_UINT32 num_bssid;
    /** bssid list         */ 
    wmi_mac_addr bssid_list[1];
} wmi_bssid_list;

/**
 * TLV for  ie data. 
 */
typedef struct {
    /** WMI_IE_TAG */
    A_UINT32     tag;  
    /** number of bytes in ie data   */
    A_UINT32 ie_len;
    /** ie data array  (ie_len adjusted to  number of words  (ie_len + 4)/4 )  */ 
    A_UINT32 ie_data[1];
} wmi_ie_data;


typedef struct {
    /** Len of the SSID */
    A_UINT32     ssid_len;
    /** SSID */
    A_UINT32     ssid[8];
} wmi_ssid;

typedef struct {
    /** WMI_SSID_LIST_TAG */
    A_UINT32     tag; 
    A_UINT32     num_ssids;
    wmi_ssid ssids[1];
} wmi_ssid_list;

/* prefix used by scan requestor ids on the host */
#define WMI_HOST_SCAN_REQUESTOR_ID_PREFIX 0xA000 
/* prefix used by scan request ids generated on the host */
/* host cycles through the lower 12 bits to generate ids */ 
#define WMI_HOST_SCAN_REQ_ID_PREFIX 0xA000 

#define WLAN_SCAN_PARAMS_MAX_SSID    16
#define WLAN_SCAN_PARAMS_MAX_BSSID   4
#define WLAN_SCAN_PARAMS_MAX_IE_LEN  256

typedef struct {
    /** Scan ID */
    A_UINT32 scan_id;
    /** Scan requestor ID */
    A_UINT32 scan_req_id;
    /** VDEV id(interface) that is requesting scan */
    A_UINT32 vdev_id;  
    /** Scan Priority, input to scan scheduler */
    A_UINT32 scan_priority;
    /** Scan events subscription */
    A_UINT32 notify_scan_events;
    /** dwell time in msec on active channels */
    A_UINT32 dwell_time_active;    
    /** dwell time in msec on passive channels */
    A_UINT32 dwell_time_passive;
    /** min time in msec on the BSS channel,only valid if atleast one VDEV is active*/
    A_UINT32 min_rest_time; 
    /** max rest time in msec on the BSS channel,only valid if at least one VDEV is active*/
    /** the scanner will rest on the bss channel at least min_rest_time. after min_rest_time the scanner
     *  will start checking for tx/rx activity on all VDEVs. if there is no activity the scanner will
     *  switch to off channel. if there is activity the scanner will let the radio on the bss channel
     *  until max_rest_time expires.at max_rest_time scanner will switch to off channel 
     *  irrespective of activity. activity is determined by the idle_time parameter.
     */
    A_UINT32  max_rest_time;          
    /** time before sending next set of probe requests. 
     *   The scanner keeps repeating probe requests transmission with period specified by repeat_probe_time.
     *   The number of probe requests specified depends on the ssid_list and bssid_list
     */
    A_UINT32  repeat_probe_time; 
    /** time in msec between 2 consequetive probe requests with in a set. */ 
    A_UINT32  probe_spacing_time; 
    /** data inactivity time in msec on bss channel that will be used by scanner for measuring the inactivity  */
    A_UINT32 idle_time;   
    /** maximum time in msec allowed for scan  */
    A_UINT32  max_scan_time;          
    /** delay in msec before sending first probe request after switching to a channel */
    A_UINT32  probe_delay;   
    /** Scan control flags */
    A_UINT32 scan_ctrl_flags;
    /** Burst duration time in msec*/
    A_UINT32 burst_duration;
    /**
     * TLV (tag length value )  paramerters follow the scan_cmd structure.
     * TLV can contain channel list, bssid list, ssid list and 
     * ie. the TLV tags are defined above; 
     */
} wmi_start_scan_cmd;

/**
 * scan control flags.
 */

/** passively scan all channels including active channels */
#define WMI_SCAN_FLAG_PASSIVE        0x1 
/** add wild card ssid probe request even though ssid_list is specified. */ 
#define WMI_SCAN_ADD_BCAST_PROBE_REQ 0x2 
/** add cck rates to rates/xrate ie for the generated probe request */ 
#define WMI_SCAN_ADD_CCK_RATES 0x4 
/** add ofdm rates to rates/xrate ie for the generated probe request */ 
#define WMI_SCAN_ADD_OFDM_RATES 0x8 
/** To enable indication of Chan load and Noise floor to host */ 
#define WMI_SCAN_CHAN_STAT_EVENT 0x10
/** Filter Probe request frames  */
#define WMI_SCAN_FILTER_PROBE_REQ 0x20

/** WMI_SCAN_CLASS_MASK must be the same value as IEEE80211_SCAN_CLASS_MASK */
#define WMI_SCAN_CLASS_MASK 0xFF000000

/*
* Masks identifying types/ID of scans
* Scan_Stop macros should be the same value as below defined in UMAC
* #define IEEE80211_SPECIFIC_SCAN       0x00000000
* #define IEEE80211_VAP_SCAN            0x01000000
* #define IEEE80211_ALL_SCANS           0x04000000
*/
#define WMI_SCAN_STOP_ONE 0x00000000
#define WMI_SCN_STOP_VAP_ALL 0x01000000
#define WMI_SCAN_STOP_ALL 0x04000000

typedef struct {
    /** requestor requesting cancel  */
    A_UINT32 requestor;
    /** Scan ID */
    A_UINT32 scan_id;
    /** 
    * Req Type 
    * req_type should be WMI_SCAN_STOP_ONE, WMI_SCN_STOP_VAP_ALL or WMI_SCAN_STOP_ALL 
    * WMI_SCAN_STOP_ONE indicates to stop a specific scan with scan_id
    * WMI_SCN_STOP_VAP_ALL indicates to stop all scan requests on a specific vDev with vdev_id
    * WMI_SCAN_STOP_ALL indicates to stop all scan requests in both Scheduler's queue and Scan Engine
    */
    A_UINT32 req_type;
    /** 
    * vDev ID 
    * used when req_type equals to WMI_SCN_STOP_VAP_ALL, it indexed the vDev on which to stop the scan
    */
    A_UINT32 vdev_id;
} wmi_stop_scan_cmd;

typedef struct {
    A_UINT32 num_scan_chans;
    wmi_channel chan_info[1]; 
} wmi_scan_chan_list_cmd;

/* Five Levels for Requested Priority */
/* VERY_LOW LOW  MEDIUM   HIGH  VERY_HIGH */
typedef A_UINT32 WLAN_PRIORITY_MAPPING[5];

/** 
* to keep align with UMAC implementation, we pass only vdev_type but not vdev_subtype when we overwrite an entry for a specific vdev_subtype
* ex. if we need overwrite P2P Client prority entry, we will overwrite the whole table for WLAN_M_STA
* we will generate the new WLAN_M_STA table with modified P2P Client Entry but keep STA entry intact
*/
typedef struct {
    /** 
    * used as an index to find the proper table for a specific vdev type in default_scan_priority_mapping_table
    * vdev_type should be one of enum in WLAN_OPMODE which inculdes WLAN_M_IBSS, WLAN_M_STA, WLAN_M_AP and WLAN_M_MONITOR currently
    */
    A_UINT32      vdev_type;
    /** 
    * number of rows in mapping_table for a specific vdev
    * for WLAN_M_STA type, there are 3 entries in the table (refer to default_scan_priority_mapping_table definition)
    */
    A_UINT32                    number_rows;
    /**  mapping_table for a specific vdev */
    WLAN_PRIORITY_MAPPING mapping_table[1];
}wmi_scan_sch_priority_table_cmd;

enum wmi_scan_event_type {
    WMI_SCAN_EVENT_STARTED=0x1,
    WMI_SCAN_EVENT_COMPLETED=0x2,
    WMI_SCAN_EVENT_BSS_CHANNEL=0x4,
    WMI_SCAN_EVENT_FOREIGN_CHANNEL = 0x8,
    WMI_SCAN_EVENT_DEQUEUED=0x10,       /* scan request got dequeued */
    WMI_SCAN_EVENT_PREEMPTED=0x20,		/* preempted by other high priority scan */
    WMI_SCAN_EVENT_START_FAILED=0x40,   /* scan start failed */
    WMI_SCAN_EVENT_RESTARTED=0x80,    /*scan restarted*/
    WMI_SCAN_EVENT_MAX=0x8000
};

enum wmi_scan_completion_reason {
    /** scan related events */
    WMI_SCAN_REASON_COMPLETED,
    WMI_SCAN_REASON_CANCELLED,
    WMI_SCAN_REASON_PREEMPTED,
    WMI_SCAN_REASON_TIMEDOUT,
    WMI_SCAN_REASON_MAX,
};

typedef struct {
    /** scan event (wmi_scan_event_type) */
    A_UINT32 event;
    /** status of the scan completion event */
    A_UINT32 reason;
    /** channel freq , only valid for FOREIGN channel event*/
    A_UINT32 channel_freq;
    /**id of the requestor whose scan is in progress */
    A_UINT32 requestor;
    /**id of the scan that is in progress */
    A_UINT32 scan_id;
    /**id of VDEV that requested the scan */
    A_UINT32 vdev_id;
} wmi_scan_event;

/*
 * This defines how much headroom is kept in the
 * receive frame between the descriptor and the
 * payload, in order for the WMI PHY error and
 * management handler to insert header contents.
 *
 * This is in bytes.
 */
#define WMI_MGMT_RX_HDR_HEADROOM    (52)

/** This event will be used for sending scan results
 * as well as rx mgmt frames to the host. The rx buffer
 * will be sent as part of this WMI event. It would be a 
 * good idea to pass all the fields in the RX status
 * descriptor up to the host.
 */
typedef struct {
    /** channel on which this frame is received. */
    A_UINT32     channel;
    /** snr information used to cal rssi */
    A_UINT32     snr;
    /** Rate kbps */
    A_UINT32     rate;
    /** rx phy mode WLAN_PHY_MODE */
    A_UINT32     phy_mode;
    /** length of the frame */
    A_UINT32     buf_len;
    /** rx status */
    A_UINT32     status;
} wmi_mgmt_rx_hdr;

typedef struct {
    /** management header */
    wmi_mgmt_rx_hdr hdr;
    /** management frame buffer */
    A_UINT8 bufp[1];
} wmi_mgmt_rx_event;

/* WMI PHY Error RX */

typedef struct {
    /** TSF timestamp */
    A_UINT32 tsf_timestamp;

    /**
     * Current freq1, freq2
     *
     * [7:0]:    freq1[lo]
     * [15:8] :   freq1[hi]
     * [23:16]:   freq2[lo]
     * [31:24]:   freq2[hi]
     */
    A_UINT32 freq_info_1;

    /**
     * Combined RSSI over all chains and channel width for this PHY error
     *
     * [7:0]: RSSI combined
     * [15:8]: Channel width (MHz)
     * [23:16]: PHY error code
     * [24:16]: reserved (future use)
     */
    A_UINT32 freq_info_2;

    /**
     * RSSI on chain 0 through 3
     *
     * This is formatted the same as the PPDU_START RX descriptor
     * field:
     *
     * [7:0]:   pri20
     * [15:8]:  sec20
     * [23:16]: sec40
     * [31:24]: sec80
     */
    A_UINT32 rssi_chain0;
    A_UINT32 rssi_chain1;
    A_UINT32 rssi_chain2;
    A_UINT32 rssi_chain3;

   /**
     * Last calibrated NF value for chain 0 through 3
     *
     * nf_list_1:
     *
     * + [15:0] - chain 0
     * + [31:16] - chain 1
     *
     * nf_list_2:
     *
     * + [15:0] - chain 2
     * + [31:16] - chain 3
     */
    A_UINT32 nf_list_1;
    A_UINT32 nf_list_2;

    /** Length of the frame */
    A_UINT32 buf_len;
} wmi_single_phyerr_rx_hdr;

#define WMI_UNIFIED_FREQINFO_1_LO   0x000000ff
#define WMI_UNIFIED_FREQINFO_1_LO_S 0
#define WMI_UNIFIED_FREQINFO_1_HI   0x0000ff00
#define WMI_UNIFIED_FREQINFO_1_HI_S 8
#define WMI_UNIFIED_FREQINFO_2_LO   0x00ff0000
#define WMI_UNIFIED_FREQINFO_2_LO_S 16
#define WMI_UNIFIED_FREQINFO_2_HI   0xff000000
#define WMI_UNIFIED_FREQINFO_2_HI_S 24

/*
 * Please keep in mind that these _SET macros break macro side effect
 * assumptions; don't be clever with them.
 */
#define WMI_UNIFIED_FREQ_INFO_GET(hdr, f)                                   \
            ( WMI_F_MS( (hdr)->freq_info_1,                                 \
              WMI_UNIFIED_FREQINFO_##f##_LO )                               \
              | (WMI_F_MS( (hdr)->freq_info_1,                              \
                 WMI_UNIFIED_FREQINFO_##f##_HI ) << 8) )

#define WMI_UNIFIED_FREQ_INFO_SET(hdr, f, v)                                \
        do {                                                                \
            WMI_F_RMW((hdr)->freq_info_1, (v) & 0xff,                       \
              WMI_UNIFIED_FREQINFO_##f##_LO);                               \
            WMI_F_RMW((hdr)->freq_info_1, ((v) >> 8) & 0xff,                \
                WMI_UNIFIED_FREQINFO_##f##_HI);                             \
        } while (0)

#define WMI_UNIFIED_FREQINFO_2_RSSI_COMB    0x000000ff
#define WMI_UNIFIED_FREQINFO_2_RSSI_COMB_S  0
#define WMI_UNIFIED_FREQINFO_2_CHWIDTH      0x0000ff00
#define WMI_UNIFIED_FREQINFO_2_CHWIDTH_S    8
#define WMI_UNIFIED_FREQINFO_2_PHYERRCODE   0x00ff0000
#define WMI_UNIFIED_FREQINFO_2_PHYERRCODE_S 16

#define WMI_UNIFIED_RSSI_COMB_GET(hdr)                                      \
            ( (int8_t) (WMI_F_MS((hdr)->freq_info_2,                        \
                WMI_UNIFIED_FREQINFO_2_RSSI_COMB)))

#define WMI_UNIFIED_RSSI_COMB_SET(hdr, v)                                   \
            WMI_F_RMW((hdr)->freq_info_2, (v) & 0xff,                       \
              WMI_UNIFIED_FREQINFO_2_RSSI_COMB);

#define WMI_UNIFIED_CHWIDTH_GET(hdr)                                        \
            WMI_F_MS((hdr)->freq_info_2, WMI_UNIFIED_FREQINFO_2_CHWIDTH)

#define WMI_UNIFIED_CHWIDTH_SET(hdr, v)                                     \
            WMI_F_RMW((hdr)->freq_info_2, (v) & 0xff,                       \
              WMI_UNIFIED_FREQINFO_2_CHWIDTH);

#define WMI_UNIFIED_PHYERRCODE_GET(hdr)                                     \
            WMI_F_MS((hdr)->freq_info_2, WMI_UNIFIED_FREQINFO_2_PHYERRCODE)

#define WMI_UNIFIED_PHYERRCODE_SET(hdr, v)                                  \
            WMI_F_RMW((hdr)->freq_info_2, (v) & 0xff,                       \
              WMI_UNIFIED_FREQINFO_2_PHYERRCODE);

#define WMI_UNIFIED_CHAIN_0     0x0000ffff
#define WMI_UNIFIED_CHAIN_0_S   0
#define WMI_UNIFIED_CHAIN_1     0xffff0000
#define WMI_UNIFIED_CHAIN_1_S   16
#define WMI_UNIFIED_CHAIN_2     0x0000ffff
#define WMI_UNIFIED_CHAIN_2_S   0
#define WMI_UNIFIED_CHAIN_3     0xffff0000
#define WMI_UNIFIED_CHAIN_3_S   16

#define WMI_UNIFIED_CHAIN_0_FIELD   nf_list_1
#define WMI_UNIFIED_CHAIN_1_FIELD   nf_list_1
#define WMI_UNIFIED_CHAIN_2_FIELD   nf_list_2
#define WMI_UNIFIED_CHAIN_3_FIELD   nf_list_2

#define WMI_UNIFIED_NF_CHAIN_GET(hdr, c)                                    \
            ((int16_t) (WMI_F_MS((hdr)->WMI_UNIFIED_CHAIN_##c##_FIELD,      \
              WMI_UNIFIED_CHAIN_##c)))

#define WMI_UNIFIED_NF_CHAIN_SET(hdr, c, nf)                                \
            WMI_F_RMW((hdr)->WMI_UNIFIED_CHAIN_##c##_FIELD, (nf) & 0xffff,  \
              WMI_UNIFIED_CHAIN_##c);

/*
 * For now, this matches what the underlying hardware is doing.
 * Update ar6000ProcRxDesc() to use these macros when populating
 * the rx descriptor and then we can just copy the field over
 * to the WMI PHY notification without worrying about breaking
 * things.
 */
#define WMI_UNIFIED_RSSI_CHAN_PRI20     0x000000ff
#define WMI_UNIFIED_RSSI_CHAN_PRI20_S   0
#define WMI_UNIFIED_RSSI_CHAN_SEC20     0x0000ff00
#define WMI_UNIFIED_RSSI_CHAN_SEC20_S   8
#define WMI_UNIFIED_RSSI_CHAN_SEC40     0x00ff0000
#define WMI_UNIFIED_RSSI_CHAN_SEC40_S   16
#define WMI_UNIFIED_RSSI_CHAN_SEC80     0xff000000
#define WMI_UNIFIED_RSSI_CHAN_SEC80_S   24

#define WMI_UNIFIED_RSSI_CHAN_SET(hdr, c, ch, rssi)                         \
            WMI_F_RMW((hdr)->rssi_chain##c, (rssi) & 0xff,                  \
              WMI_UNIFIED_RSSI_CHAN_##ch);

#define WMI_UNIFIED_RSSI_CHAN_GET(hdr, c, ch)                               \
            ((int8_t) (WMI_F_MS((hdr)->rssi_chain##c,                       \
              WMI_UNIFIED_RSSI_CHAN_##ch)))

typedef struct {
    /** Phy error event header */
    wmi_single_phyerr_rx_hdr hdr;
    /** frame buffer */
    A_UINT8 bufp[1];
}wmi_single_phyerr_rx_event;

typedef struct {
    /** Phy error phy error count */
    A_UINT32 num_phyerr_events;
    A_UINT32 tsf_l32;
    A_UINT32 tsf_u32;
} wmi_comb_phyerr_rx_hdr;

typedef struct {
    /** Phy error phy error count */
    wmi_comb_phyerr_rx_hdr hdr;
    /** frame buffer - contains multiple payloads in the order:
     *                    header - payload, header - payload...
     *  (The header is of type: wmi_single_phyerr_rx_hdr) */
    A_UINT8 *bufp;
} wmi_comb_phyerr_rx_event;

/* WMI MGMT TX  */
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** xmit rate */
    A_UINT32 tx_rate;
    /** xmit power */
    A_UINT32 tx_power;
    /** Buffer length in bytes */
    A_UINT32 buf_len;
} wmi_mgmt_tx_hdr;

typedef struct {
    /** header */
    wmi_mgmt_tx_hdr hdr;
    /** buffer */
    A_UINT8 bufp[1];
} wmi_mgmt_tx_cmd;

typedef struct {
    A_UINT32 value;
} wmi_echo_event;

typedef struct {
    A_UINT32 value;
}wmi_echo_cmd;


typedef struct {
    /** 2 char iso name byte 0 contains second char and  
        byte 1 contains first char of the country code name */
    A_UINT32 country_code;
} wmi_pdev_set_country_cmd;

typedef struct {
    /** reg domain code */
    A_UINT32 reg_domain;
    A_UINT32 reg_domain_2G;
    A_UINT32 reg_domain_5G;
    A_UINT32 conformance_test_limit_2G;
    A_UINT32 conformance_test_limit_5G;
} wmi_pdev_set_regdomain_cmd;

typedef struct {
    /** TRUE for scan start and flase for scan end */
    A_UINT32 scan_start; 
} wmi_pdev_scan_cmd;

//currently, only RTT measurement has been implemented

/*
 * Mesage format for WMI_RTT_TSF_CMDID
 * This CMD trigger FW to report TSF Measurement result to host
 */
typedef struct {
    A_UINT32 req_id;               //unique request ID for this TSF measure req
    wmi_channel channel;          //channel information for this Requirement
    wmi_mac_addr dest_mac;        //destination mac address for measurement
    wmi_mac_addr spoof_bssid;     //spoof BSSID for measurement with unassociated STA
    A_UINT32 vdev_id;              // vdev used for TSF
    A_UINT32 time_out;             //timeout for this TSF mesurement (ms)
}wmi_rtt_tsf_cmd;

/*
 * Mesage format for WMI_RTT_MEASREQ_CMDID
 * This CMD trigger FW to start measurement with a peer
 * Need be careful about 32 alignment if any change made in future
 */
typedef struct { //notice on 32 bit alignment if need do any further change
    A_UINT32 req_id;                //unique request ID for this RTT measure req
    A_UINT32 sta_num;               // how many number of STA in this RTT requirement
    A_UINT32 flag;                  // current bit 0 indicate channel switch or not, all other reserved
    wmi_channel channel;            // common channel information for this Requirement
} wmi_rtt_measreq_head_chan;

typedef struct { //notice on 32 bit alignment if need do any further change
    A_UINT32 req_id;                //unique request ID for this RTT measure req
    A_UINT32 sta_num;               // how many number of STA in this RTT requirement
    A_UINT32 flag;                  // current bit 0 indicate channel switch or not, all other reserved
} wmi_rtt_measreq_head;
    
typedef struct { //any new change need take care of 32 alignment
    A_UINT32 control_flag;       // some control information here
  /*********************************************************************************
   *Bits 1:0: Reserved
   *Bits 4:2: 802.11 Frame Type to measure RTT 
   *000: NULL, 001: Qos NULL, 010: TMR-TM (It is HOST's responsibility to choose the 
   *right Frame type (peer should support))
   *Bits 8:5: Transmit chainmask to use for transmission 0000 - 1111 
   *Bits 12:9: Receive chainmask to use for reception 0000 - 1111
   *Bits 13: 802.11v proprietary protocol support
   *Bits 15:14: BW 0 -Legacy20MHz 1- HT/VHT20MHz 2- HT/VHT40MHz 3 - VHT80 MHz
   *Bits 17:16: Preamble 0- Legacy 2-HT 3-VHT
   *Bits 21:18: Retry times
   *Bits 29:22  rates info
   *    The rates used are based on the Rate Control Table (HW MAC rate value).
   *    31:0: IEEE Physical Layer Transmission Rate of the Probe frame
   *    MCS 0= 0x80 MCS 1=0x81......MCS 15 = 0x8f
   *    Legacy 6 Mb/s 0x0b
   *    9Mb/s: 0x0f
   *    12 Mb/s: 0x0a
   *    18Mb/s: 0x0e
   *    24Mb/s: 0x09
   *    36 Mb/s: 0x0d
   *    48 Mb/s: 0x08
   *    54 Mb/s: 0x0c
   * Bits 31:30 Reserved
   *********************************************************************************/
    A_UINT32 measure_info;
    /*******************************************************************************
     *Bit 0-7 vdev_id vdev used for RTT
     *Bit 15-8 num_meas #of measurements of each peer
     *Bit 23:16 timeout for this rtt mesurement (ms)
     *Bit 31-24 report_type  not support in this version, for future
     *******************************************************************************/
    wmi_mac_addr dest_mac;      //destination mac address for measurement
    wmi_mac_addr spoof_bssid;   //spoof BSSID for measurement with unassociated STA
    //we can expand dest_mac and spoof_mac after here(according to sta_num)
}wmi_rtt_measreq_body;
/*---end of RTT COMMAND---*/
 
/*Command to set/unset chip in quiet mode*/
typedef struct {
	A_UINT32 period;		/*period in TUs*/	
	A_UINT32 duration;		/*duration in TUs*/
	A_UINT32 next_start;	/*offset in TUs*/
        A_UINT32 enabled; 		/*enable/disable*/
} wmi_pdev_set_quiet_cmd;

/*
 * Command to enable/disable Green AP Power Save.
 * This helps conserve power during AP operation. When the AP has no
 * stations associated with it, the host can enable Green AP Power Save
 * to request the firmware to shut down all but one transmit and receive
 * chains.
 */
typedef struct {
    A_UINT32 enable; 		 /*1:enable, 0:disable*/
} wmi_pdev_green_ap_ps_enable_cmd;


#define MAX_HT_IE_LEN 32
typedef struct {
    A_UINT32 ie_len; 		 /*length of the ht ie */
    A_UINT32 ie_data[1]; 	 /*length of the ht ie data */
} wmi_pdev_set_ht_ie_cmd;

#define MAX_VHT_IE_LEN 32
typedef struct {
    A_UINT32 ie_len; 		 /*length of the vht ie */
    A_UINT32 ie_data[1]; 	 /*length of the vht ie data */
} wmi_pdev_set_vht_ie_cmd;

typedef struct {
    wmi_mac_addr base_macaddr;
} wmi_pdev_set_base_macaddr_cmd;

/*
 * For now, the spectral configuration is global rather than
 * per-vdev.  The vdev is a placeholder and will be ignored
 * by the firmware.
 */
typedef struct {
        A_UINT32    vdev_id;
        A_UINT32    spectral_scan_count;
        A_UINT32    spectral_scan_period;
        A_UINT32    spectral_scan_priority;
        A_UINT32    spectral_scan_fft_size;
        A_UINT32    spectral_scan_gc_ena;
        A_UINT32    spectral_scan_restart_ena;
        A_UINT32    spectral_scan_noise_floor_ref;
        A_UINT32    spectral_scan_init_delay;
        A_UINT32    spectral_scan_nb_tone_thr;
        A_UINT32    spectral_scan_str_bin_thr;
        A_UINT32    spectral_scan_wb_rpt_mode;
        A_UINT32    spectral_scan_rssi_rpt_mode;
        A_UINT32    spectral_scan_rssi_thr;
        A_UINT32    spectral_scan_pwr_format;
        A_UINT32    spectral_scan_rpt_mode;
        A_UINT32    spectral_scan_bin_scale;
        A_UINT32    spectral_scan_dBm_adj;
        A_UINT32    spectral_scan_chn_mask;
} wmi_vdev_spectral_configure_cmd;

/*
 * Enabling, disabling and triggering the spectral scan
 * is a per-vdev operation.  That is, it will set channel
 * flags per vdev rather than globally; so concurrent scan/run
 * and multiple STA (eg p2p, tdls, multi-band STA) is possible.
 */
typedef struct {
    A_UINT32    vdev_id;
    /* 0 - ignore; 1 - trigger, 2 - clear trigger */
    A_UINT32    trigger_cmd;
    /* 0 - ignore; 1 - enable, 2 - disable */
    A_UINT32    enable_cmd;
} wmi_vdev_spectral_enable_cmd;

typedef enum {
WMI_CSA_IE_PRESENT = 0x00000001,
WMI_XCSA_IE_PRESENT = 0x00000002,
WMI_WBW_IE_PRESENT = 0x00000004,
WMI_CSWARP_IE_PRESENT = 0x00000008,
}WMI_CSA_EVENT_IES_PRESENT_FLAG;

/* wmi CSA receive event from beacon frame */
typedef struct{
    A_UINT32 i_fc_dur;
//  Bit 0-15: FC
//  Bit 16-31: DUR  
    wmi_mac_addr i_addr1;
    wmi_mac_addr i_addr2;
    A_UINT32 csa_ie[2];
    A_UINT32 xcsa_ie[2];
    A_UINT32 wb_ie[2];
    A_UINT32 cswarp_ie;
    A_UINT32 ies_present_flag; //WMI_CSA_EVENT_IES_PRESENT_FLAG
}wmi_csa_event;

typedef enum {
    /** TX chian mask */
    WMI_PDEV_PARAM_TX_CHAIN_MASK = 0x1,
    /** RX chian mask */
    WMI_PDEV_PARAM_RX_CHAIN_MASK,
    /** TX power limit for 2G Radio */
    WMI_PDEV_PARAM_TXPOWER_LIMIT2G,
    /** TX power limit for 5G Radio */
    WMI_PDEV_PARAM_TXPOWER_LIMIT5G,
    /** TX power scale */
    WMI_PDEV_PARAM_TXPOWER_SCALE,
    /** Beacon generation mode . 0: host, 1: target   */
    WMI_PDEV_PARAM_BEACON_GEN_MODE,
    /** Beacon generation mode . 0: staggered 1: bursted   */
    WMI_PDEV_PARAM_BEACON_TX_MODE,
    /** Resource manager off chan mode . 
     * 0: turn off off chan mode. 1: turn on offchan mode 
     */
    WMI_PDEV_PARAM_RESMGR_OFFCHAN_MODE,
    /** Protection mode  0: no protection 1:use CTS-to-self 2: use RTS/CTS */
    WMI_PDEV_PARAM_PROTECTION_MODE,
    /** Dynamic bandwidth 0: disable 1: enable */
    WMI_PDEV_PARAM_DYNAMIC_BW,
    /** Non aggregrate/ 11g sw retry threshold.0-disable */
    WMI_PDEV_PARAM_NON_AGG_SW_RETRY_TH,
    /** aggregrate sw retry threshold. 0-disable*/
    WMI_PDEV_PARAM_AGG_SW_RETRY_TH,
    /** Station kickout threshold (non of consecutive failures).0-disable */
    WMI_PDEV_PARAM_STA_KICKOUT_TH,
    /** Aggerate size scaling configuration per AC */
    WMI_PDEV_PARAM_AC_AGGRSIZE_SCALING,
    /** LTR enable */
    WMI_PDEV_PARAM_LTR_ENABLE,
    /** LTR latency for BE, in us */
    WMI_PDEV_PARAM_LTR_AC_LATENCY_BE,
    /** LTR latency for BK, in us */
    WMI_PDEV_PARAM_LTR_AC_LATENCY_BK,
    /** LTR latency for VI, in us */
    WMI_PDEV_PARAM_LTR_AC_LATENCY_VI,
    /** LTR latency for VO, in us  */
    WMI_PDEV_PARAM_LTR_AC_LATENCY_VO,
    /** LTR AC latency timeout, in ms */
    WMI_PDEV_PARAM_LTR_AC_LATENCY_TIMEOUT,
    /** LTR platform latency override, in us */
    WMI_PDEV_PARAM_LTR_SLEEP_OVERRIDE,
    /** LTR-M override, in us */
    WMI_PDEV_PARAM_LTR_RX_OVERRIDE,
    /** Tx activity timeout for LTR, in us */
    WMI_PDEV_PARAM_LTR_TX_ACTIVITY_TIMEOUT,
    /** L1SS state machine enable */
    WMI_PDEV_PARAM_L1SS_ENABLE,
    /** Deep sleep state machine enable */
    WMI_PDEV_PARAM_DSLEEP_ENABLE,
    /** RX buffering flush enable */
    WMI_PDEV_PARAM_PCIELP_TXBUF_FLUSH,
    /** RX buffering matermark */
    WMI_PDEV_PARAM_PCIELP_TXBUF_WATERMARK,
    /** RX buffering timeout enable */
    WMI_PDEV_PARAM_PCIELP_TXBUF_TMO_EN,
    /** RX buffering timeout value */
    WMI_PDEV_PARAM_PCIELP_TXBUF_TMO_VALUE,
    /** pdev level stats update period in ms */
    WMI_PDEV_PARAM_PDEV_STATS_UPDATE_PERIOD,
    /** vdev level stats update period in ms */
    WMI_PDEV_PARAM_VDEV_STATS_UPDATE_PERIOD,
    /** peer level stats update period in ms */
    WMI_PDEV_PARAM_PEER_STATS_UPDATE_PERIOD,
    /** beacon filter status update period */
    WMI_PDEV_PARAM_BCNFLT_STATS_UPDATE_PERIOD,
    /** QOS Mgmt frame protection MFP/PMF 0: disable, 1: enable */
    WMI_PDEV_PARAM_PMF_QOS,
    /** Access category on which ARP frames are sent */
    WMI_PDEV_PARAM_ARP_AC_OVERRIDE,
    /** DCS configuration */
    WMI_PDEV_PARAM_DCS,
	/** Enable/Disable ANI on target */ 
    WMI_PDEV_PARAM_ANI_ENABLE,
    /** Enable/Disable CDD for 1x1 STAs in rate control module */
    WMI_PDEV_PARAM_DYNTXCHAIN,
    /** Enable/Disable proxy STA */
    WMI_PDEV_PARAM_PROXY_STA,
    /** Enable/Disable low power state when all VDEVs are inactive/idle. */
    WMI_PDEV_PARAM_IDLE_PS_CONFIG,
    /** Enable/Disable power gating sleep */
    WMI_PDEV_PARAM_POWER_GATING_SLEEP,
} WMI_PDEV_PARAM;

typedef struct {
    /** parameter id   */
    A_UINT32 param_id;
    /** parametr value */
    A_UINT32 param_value;
} wmi_pdev_set_param_cmd;
typedef struct {
    /** parameter   */
    A_UINT32 param;
} wmi_pdev_get_tpc_config_cmd;

#define WMI_TPC_RATE_MAX            160 
#define WMI_TPC_TX_NUM_CHAIN        4

typedef enum {
    WMI_TPC_CONFIG_EVENT_FLAG_TABLE_CDD     = 0x1,
    WMI_TPC_CONFIG_EVENT_FLAG_TABLE_STBC    = 0x2,
    WMI_TPC_CONFIG_EVENT_FLAG_TABLE_TXBF    = 0x4,
} WMI_TPC_CONFIG_EVENT_FLAG;

typedef struct {
    A_UINT32 regDomain;
    A_UINT32 chanFreq;
    A_UINT32 phyMode;
    A_UINT32 twiceAntennaReduction;
    A_UINT32 twiceMaxRDPower;
    A_INT32  twiceAntennaGain;
    A_UINT32 powerLimit;
    A_UINT32 rateMax;
    A_UINT32 numTxChain;
    A_UINT32 ctl;
    A_UINT32 flags;
    A_INT8  maxRegAllowedPower[WMI_TPC_TX_NUM_CHAIN];   
    A_INT8  maxRegAllowedPowerAGCDD[WMI_TPC_TX_NUM_CHAIN][WMI_TPC_TX_NUM_CHAIN];   
    A_INT8  maxRegAllowedPowerAGSTBC[WMI_TPC_TX_NUM_CHAIN][WMI_TPC_TX_NUM_CHAIN];   
    A_INT8  maxRegAllowedPowerAGTXBF[WMI_TPC_TX_NUM_CHAIN][WMI_TPC_TX_NUM_CHAIN];   
    A_UINT8 ratesArray[WMI_TPC_RATE_MAX];
} wmi_pdev_tpc_config_event;


/*
 * Transmit power scale factor.
 *
 */
typedef enum {
    WMI_TP_SCALE_MAX    = 0,        /* no scaling (default) */
    WMI_TP_SCALE_50     = 1,        /* 50% of max (-3 dBm) */
    WMI_TP_SCALE_25     = 2,        /* 25% of max (-6 dBm) */
    WMI_TP_SCALE_12     = 3,        /* 12% of max (-9 dBm) */
    WMI_TP_SCALE_MIN    = 4,        /* min, but still on   */
    WMI_TP_SCALE_SIZE   = 5,        /* max num of enum     */
} WMI_TP_SCALE;

typedef struct {
    /* channel (only frequency and mode info are used) */ 
    wmi_channel chan; 
} wmi_set_channel_cmd;

typedef struct {
   /** number of channels */
   A_UINT32 num_chan;
   /** array of channels */
   wmi_channel  channel_list[1];
}  wmi_pdev_chanlist_update_event;

#define WMI_MAX_DEBUG_MESG (sizeof(A_UINT32) * 32) 
typedef struct {
    struct dbglog_config_msg_s config;
}WMI_DBGLOG_CFG_CMD;

typedef struct {
   /** message buffer, NULL terminated */
   char bufp[WMI_MAX_DEBUG_MESG]; 
} wmi_debug_mesg_event;

enum {
    /** IBSS station */
    VDEV_TYPE_IBSS  = 0,
    /** infra STA */
    VDEV_TYPE_STA   = 1,
    /** infra AP */
    VDEV_TYPE_AP    = 2,
    /** Monitor */
    VDEV_TYPE_MONITOR =3,
};

enum {
    /** P2P device */
    VDEV_SUBTYPE_P2PDEV=0,
    /** P2P client */
    VDEV_SUBTYPE_P2PCLI,
    /** P2P GO */
    VDEV_SUBTYPE_P2PGO,
    /** BT3.0 HS */
    VDEV_SUBTYPE_BT,
};

typedef struct {
    /** idnore power , only use flags , mode and freq */
   wmi_channel  chan;
}    wmi_pdev_set_channel_cmd;

typedef enum {
    WMI_PKTLOG_EVENT_RX  = 0x1,
    WMI_PKTLOG_EVENT_TX  = 0x2,
    WMI_PKTLOG_EVENT_RCF = 0x4, /* Rate Control Find */
    WMI_PKTLOG_EVENT_RCU = 0x8, /* Rate Control Update */
} WMI_PKTLOG_EVENT;

typedef PREPACK struct {
    WMI_PKTLOG_EVENT evlist;
} POSTPACK wmi_pdev_pktlog_enable_cmd;

/** Customize the DSCP (bit) to TID (0-7) mapping for QOS */
#define WMI_DSCP_MAP_MAX    (64)
    /*
     * @brief dscp_tid_map_cmdid - command to send the dscp to tid map to the target
     * @details
     * Create an API for sending the custom DSCP-to-TID map to the target
     * If this is a request from the user space or from above the UMAC
     * then the best place to implement this is in the umac_if_offload of the OL path.
     * Provide a place holder for this API in the ieee80211com (ic). 
     *
     * This API will be a function pointer in the ieee80211com (ic). Any user space calls for manually setting the DSCP-to-TID mapping
     * in the target should be directed to the function pointer in the ic.
     *
     * Implementation details of the API to send the map to the target are as described-
     * 
     * 1. The function will have 2 arguments- struct ieee80211com, DSCP-to-TID map.
     *    DSCP-to-TID map is a one dimensional u_int32_t array of length 64 to accomodate 
     *    64 TID values for 2^6 (64) DSCP ids.
     *    Example:
     *      A_UINT32 dscp_tid_map[WMI_DSCP_MAP_MAX] = {
	 *									0, 0, 0, 0, 0, 0, 0, 0,
	 *									1, 1, 1, 1, 1, 1, 1, 1,
	 *									2, 2, 2, 2, 2, 2, 2, 2,
	 *									3, 3, 3, 3, 3, 3, 3, 3,
	 *									4, 4, 4, 4, 4, 4, 4, 4,
	 *									5, 5, 5, 5, 5, 5, 5, 5,
	 *									6, 6, 6, 6, 6, 6, 6, 6,
	 *									7, 7, 7, 7, 7, 7, 7, 7,
	 *								  };
     * 
     * 2. Request for the WMI buffer of size equal to the size of the DSCP-to-TID map.
     * 
     * 3. Copy the DSCP-to-TID map into the WMI buffer.
     *
     * 4. Invoke the wmi_unified_cmd_send to send the cmd buffer to the target with the
     *    WMI_PDEV_SET_DSCP_TID_MAP_CMDID. Arguments to the wmi send cmd API 
     *    (wmi_unified_send_cmd) are wmi handle, cmd buffer, length of the cmd buffer and 
     *    the WMI_PDEV_SET_DSCP_TID_MAP_CMDID id.
     */
typedef struct {
    /** map indicating DSCP to TID conversion */
    A_UINT32 dscp_to_tid_map[WMI_DSCP_MAP_MAX];
} wmi_pdev_set_dscp_tid_map_cmd;

/** Fixed rate (rate-code) for broadcast/ multicast data frames */
/* @brief bcast_mcast_data_rate - set the rates for the bcast/ mcast frames
 * @details
 * Create an API for setting the custom rate for the MCAST and BCAST frames 
 * in the target. If this is a request from the user space or from above the UMAC
 * then the best place to implement this is in the umac_if_offload of the OL path.
 * Provide a place holder for this API in the ieee80211com (ic). 
 *
 * Implementation details of the API to set custom rates for MCAST and BCAST in
 * the target are as described-
 * 
 * 1. The function will have 3 arguments- 
 *    vap structure, 
 *    MCAST/ BCAST identifier code, 
 *    8 bit rate code
 *
 * The rate-code is a 1-byte field in which:for given rate, nss and preamble
 * b'7-b-6 indicate the preamble (0 OFDM, 1 CCK, 2, HT, 3 VHT)
 * b'5-b'4 indicate the NSS (0 - 1x1, 1 - 2x2, 2 - 3x3)
 * b'3-b'0 indicate the rate, which is indicated as follows:
 *          OFDM :     0: OFDM 48 Mbps
 *                     1: OFDM 24 Mbps
 *                     2: OFDM 12 Mbps
 *                     3: OFDM 6 Mbps
 *                     4: OFDM 54 Mbps
 *                     5: OFDM 36 Mbps
 *                     6: OFDM 18 Mbps
 *                     7: OFDM 9 Mbps
 *         CCK (pream == 1)
 *                     0: CCK 11 Mbps Long
 *                     1: CCK 5.5 Mbps Long
 *                     2: CCK 2 Mbps Long
 *                     3: CCK 1 Mbps Long
 *                     4: CCK 11 Mbps Short
 *                     5: CCK 5.5 Mbps Short
 *                     6: CCK 2 Mbps Short
 *         HT/VHT (pream == 2/3)
 *                     0..7: MCS0..MCS7 (HT)
 *                     0..9: MCS0..MCS9 (VHT)
 * 
 * 2. Invoke the wmi_unified_vdev_set_param_send to send the rate value
 *    to the target.
 *    Arguments to the API are-
 *    wmi handle, 
 *    VAP interface id (av_if_id) defined in ol_ath_vap_net80211,
 *    WMI_VDEV_PARAM_BCAST_DATA_RATE/ WMI_VDEV_PARAM_MCAST_DATA_RATE,
 *    rate value.
 */
typedef enum {
    WMI_SET_MCAST_RATE,
    WMI_SET_BCAST_RATE 
} MCAST_BCAST_RATE_ID;

typedef struct {
    MCAST_BCAST_RATE_ID rate_id;
    A_UINT32 rate;
} mcast_bcast_rate;

typedef struct {
    A_UINT32 cwmin;
    A_UINT32 cwmax;
    A_UINT32 aifs;
    A_UINT32 txoplimit;
    A_UINT32 acm;
    A_UINT32 no_ack;
} wmi_wmm_params;

typedef struct {
    wmi_wmm_params wmm_params_ac_be;
    wmi_wmm_params wmm_params_ac_bk;
    wmi_wmm_params wmm_params_ac_vi;
    wmi_wmm_params wmm_params_ac_vo;
} wmi_pdev_set_wmm_params_cmd;

typedef enum {
    WMI_REQUEST_PEER_STAT  = 0x01,
    WMI_REQUEST_AP_STAT    = 0x02
} wmi_stats_id;

typedef struct {
    wmi_stats_id stats_id;
    /*
     * Space to add parameters like 
     * peer mac addr
     */
} wmi_request_stats_cmd;

/** Suspend option */
enum {
    WMI_PDEV_SUSPEND, /* suspend */
    WMI_PDEV_SUSPEND_AND_DISABLE_INTR, /* suspend and disable all interrupts */
};
typedef struct {
    /* suspend option sent to target */
    A_UINT32 suspend_opt;
} wmi_pdev_suspend_cmd;


typedef struct {
    wmi_stats_id stats_id;
    /** number of pdev stats event structures (wmi_pdev_stats) 0 or 1 */
    A_UINT32 num_pdev_stats; 
    /** number of vdev stats event structures  (wmi_vdev_stats) 0 or max vdevs */
    A_UINT32 num_vdev_stats; 
    /** number of peer stats event structures  (wmi_peer_stats) 0 or max peers */
    A_UINT32 num_peer_stats;
    A_UINT32 num_bcnflt_stats;
    /** followed by
     *   num_pdev_stats * size of(struct wmi_pdev_stats)  
     *   num_vdev_stats * size of(struct wmi_vdev_stats)  
     *   num_peer_stats * size of(struct wmi_peer_stats)
     *  
     *  By having a zero sized array, the pointer to data area
     *  becomes available without increasing the struct size
     */
    A_UINT8 data[1];
} wmi_stats_event;

/**
 *  PDEV statistics
 *  @todo
 *  add all PDEV stats here
 */
typedef struct {
    /** Channel noise floor */
    A_INT32 chan_nf;
    /** TX frame count */
    A_UINT32 tx_frame_count;
    /** RX frame count */
    A_UINT32 rx_frame_count;
    /** rx clear count */
    A_UINT32 rx_clear_count;
    /** cycle count */
    A_UINT32 cycle_count;
    /** Phy error count */
    A_UINT32 phy_err_count;
    /** Channel Tx Power */
    A_UINT32 chan_tx_pwr;
    /** WAL dbg stats */
    struct wal_dbg_stats wal_pdev_stats;

} wmi_pdev_stats;

/**
 *  VDEV statistics
 *  @todo
 *  add all VDEV stats here
 */

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_vdev_stats;

/**
 *  peer statistics.
 *
 * @todo
 * add more stats
 *
 */
typedef struct {
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** rssi */
    A_UINT32  peer_rssi;
    /** last tx data rate used for peer */
    A_UINT32  peer_tx_rate; 
} wmi_peer_stats;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;  
    /** VDEV type (AP,STA,IBSS,MONITOR) */
    A_UINT32 vdev_type;
    /** VDEV subtype (P2PDEV, P2PCLI, P2PGO, BT3.0)*/
    A_UINT32 vdev_subtype;
    /** VDEV MAC address */
    wmi_mac_addr vdev_macaddr;
} wmi_vdev_create_cmd;

typedef struct {
    A_UINT32   type_count; /** 255: continuous schedule, 0: reserved */
    A_UINT32   duration ;  /** Absent period duration in micro seconds */
    A_UINT32   interval;   /** Absent period interval in micro seconds */
    A_UINT32   start_time; /** 32 bit tsf time when in starts */
} wmi_p2p_noa_descriptor;

/** values for vdev_type */
#define WMI_VDEV_TYPE_AP         0x1
#define WMI_VDEV_TYPE_STA        0x2
#define WMI_VDEV_TYPE_IBSS       0x3
#define WMI_VDEV_TYPE_MONITOR    0x4

/** values for vdev_subtype */
#define WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE 0x1
#define WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT 0x2
#define WMI_UNIFIED_VDEV_SUBTYPE_P2P_GO     0x3

/** values for vdev_start_request flags */
/** Indicates that AP VDEV uses hidden ssid. only valid for
 *  AP/GO */
#define WMI_UNIFIED_VDEV_START_HIDDEN_SSID  (1<<0)
/** Indicates if robust management frame/management frame
 *  protection is enabled. For GO/AP vdevs, it indicates that
 *  it may support station/client associations with RMF enabled.
 *  For STA/client vdevs, it indicates that sta will
 *  associate with AP with RMF enabled. */
#define WMI_UNIFIED_VDEV_START_PMF_ENABLED  (1<<1)

typedef struct {    
    /** WMI channel */
    wmi_channel chan;
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** requestor id identifying the caller module */
    A_UINT32 requestor_id;
    /** beacon interval from received beacon */
    A_UINT32 beacon_interval;
    /** DTIM Period from the received beacon */
    A_UINT32 dtim_period;
    /** Flags */
    A_UINT32 flags;
    /** ssid field. Only valid for AP/GO/IBSS/BTAmp VDEV type. */
    wmi_ssid ssid;
    /** beacon/probe reponse xmit rate. Applicable for SoftAP. */
    A_UINT32 bcn_tx_rate;
    /** beacon/probe reponse xmit power. Applicable for SoftAP. */
    A_UINT32 bcn_txPower;
    /** number of p2p NOA descriptor(s) from scan entry */ 
    A_UINT32  num_noa_descriptors;
    /** Disable H/W ack. This used by WMI_VDEV_RESTART_REQUEST_CMDID.
          During CAC, Our HW shouldn't ack ditected frames */ 
    A_UINT32  disable_hw_ack;
    /** actual p2p NOA descriptor from scan entry */ 
    wmi_p2p_noa_descriptor  noa_descriptors[2];
} wmi_vdev_start_request_cmd;

typedef struct {       
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_vdev_delete_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** aid (assoc id) received in association response for STA VDEV  */
    A_UINT32 vdev_assoc_id;
    /** bssid of the BSS the VDEV is joining  */
    wmi_mac_addr vdev_bssid;
} wmi_vdev_up_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_vdev_stop_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_vdev_down_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_vdev_standby_response_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_vdev_resume_response_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** parameter id   */
    A_UINT32 param_id;
    /** parameter value */
    A_UINT32 param_value;
} wmi_vdev_set_param_cmd;

typedef struct {
    A_UINT32 key_seq_counter_l;
    A_UINT32 key_seq_counter_h;
} wmi_key_seq_counter; 

#define  WMI_CIPHER_NONE     0x0  /* clear key */ 
#define  WMI_CIPHER_WEP      0x1 
#define  WMI_CIPHER_TKIP     0x2 
#define  WMI_CIPHER_AES_OCB  0x3 
#define  WMI_CIPHER_AES_CCM  0x4 
#define  WMI_CIPHER_WAPI     0x5 
#define  WMI_CIPHER_CKIP     0x6 
#define  WMI_CIPHER_AES_CMAC 0x7

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** MAC address used for installing   */
    wmi_mac_addr peer_macaddr;
    /** key index */
    A_UINT32 key_ix;
    /** key flags */
    A_UINT32 key_flags;
    /** key cipher, defined above */
    A_UINT32 key_cipher;
    /** key rsc counter */
    wmi_key_seq_counter key_rsc_counter; 
    /** global key rsc counter */
    wmi_key_seq_counter key_global_rsc_counter; 
    /** global key tsc counter */
    wmi_key_seq_counter key_tsc_counter; 
    /** WAPI key rsc counter */
    A_UINT8 wpi_key_rsc_counter[16]; 
    /** WAPI key tsc counter */
    A_UINT8 wpi_key_tsc_counter[16];
    /** key length */
    A_UINT32 key_len;
    /** key tx mic length */
    A_UINT32 key_txmic_len;
    /** key rx mic length */
    A_UINT32 key_rxmic_len;
    /* actual key data */
    A_UINT8  key_data[1]; /* contains key followed by tx mic followed by rx mic */
} wmi_vdev_install_key_cmd;

/** Preamble types to be used with VDEV fixed rate configuration */
typedef enum {
    WMI_RATE_PREAMBLE_OFDM,
    WMI_RATE_PREAMBLE_CCK,
    WMI_RATE_PREAMBLE_HT,
    WMI_RATE_PREAMBLE_VHT,
} WMI_RATE_PREAMBLE;

/** Value to disable fixed rate setting */ 
#define WMI_FIXED_RATE_NONE    (0xff)

/** the definition of different VDEV parameters */
typedef enum {
    /** RTS Threshold */
    WMI_VDEV_PARAM_RTS_THRESHOLD = 0x1,
    /** Fragmentation threshold */
    WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD,
    /** beacon interval in TUs */
    WMI_VDEV_PARAM_BEACON_INTERVAL,
    /** Listen interval in TUs */
    WMI_VDEV_PARAM_LISTEN_INTERVAL,
    /** muticast rate in Mbps */
    WMI_VDEV_PARAM_MULTICAST_RATE,
    /** management frame rate in Mbps */
    WMI_VDEV_PARAM_MGMT_TX_RATE,
    /** slot time (long vs short) */
    WMI_VDEV_PARAM_SLOT_TIME,
    /** preamble (long vs short) */
    WMI_VDEV_PARAM_PREAMBLE,
    /** SWBA time (time before tbtt in msec) */
    WMI_VDEV_PARAM_SWBA_TIME,
    /** time period for updating VDEV stats */
    WMI_VDEV_STATS_UPDATE_PERIOD,
    /** age out time in msec for frames queued for station in power save*/
    WMI_VDEV_PWRSAVE_AGEOUT_TIME,
    /** Host SWBA interval (time in msec before tbtt for SWBA event generation) */
    WMI_VDEV_HOST_SWBA_INTERVAL,
    /** DTIM period (specified in units of num beacon intervals) */
    WMI_VDEV_PARAM_DTIM_PERIOD,
    /** scheduler air time limit for this VDEV. used by off chan scheduler  */
    WMI_VDEV_OC_SCHEDULER_AIR_TIME_LIMIT,
    /** enable/dsiable WDS for this VDEV  */
    WMI_VDEV_PARAM_WDS,
    /** ATIM Window */
    WMI_VDEV_PARAM_ATIM_WINDOW,
    /** BMISS max */
    WMI_VDEV_PARAM_BMISS_COUNT_MAX,
    /** WMM enables/disabled */
    WMI_VDEV_PARAM_FEATURE_WMM,
    /** Channel width */
    WMI_VDEV_PARAM_CHWIDTH,
    /** Channel Offset */
    WMI_VDEV_PARAM_CHEXTOFFSET,
    /** Disable HT Protection */
    WMI_VDEV_PARAM_DISABLE_HTPROTECTION,
    /** Quick STA Kickout */
    WMI_VDEV_PARAM_STA_QUICKKICKOUT,
    /** Rate to be used with Management frames */
    WMI_VDEV_PARAM_MGMT_RATE,
    /** Protection Mode */
    WMI_VDEV_PARAM_PROTECTION_MODE,
    /** Fixed rate setting */
    WMI_VDEV_PARAM_FIXED_RATE,
    /** Short GI Enable/Disable */
    WMI_VDEV_PARAM_SGI,
    /** Enable LDPC */
    WMI_VDEV_PARAM_LDPC,
    /** Enable Tx STBC */
    WMI_VDEV_PARAM_TX_STBC,
    /** Enable Rx STBC */
    WMI_VDEV_PARAM_RX_STBC,
    /** Intra BSS forwarding  */
    WMI_VDEV_PARAM_INTRA_BSS_FWD,
    /** Setting Default xmit key for Vdev */
    WMI_VDEV_PARAM_DEF_KEYID,
    /** NSS width */
    WMI_VDEV_PARAM_NSS,
    /** Set the custom rate for the broadcast data frames */
    WMI_VDEV_PARAM_BCAST_DATA_RATE,
    /** Set the custom rate (rate-code) for multicast data frames */
    WMI_VDEV_PARAM_MCAST_DATA_RATE,
    /** Tx multicast packet indicate Enable/Disable */
    WMI_VDEV_PARAM_MCAST_INDICATE,
    /** Tx DHCP packet indicate Enable/Disable */
    WMI_VDEV_PARAM_DHCP_INDICATE,

    /* The minimum amount of time AP begins to consider STA inactive */
    WMI_VDEV_PARAM_AP_KEEPALIVE_MIN_IDLE_INACTIVE_TIME_SECS,

    /* An associated STA is considered inactive when there is no recent TX/RX
     * activity and no downlink frames are buffered for it. Once a STA exceeds
     * the maximum idle inactive time, the AP will send an 802.11 data-null as
     * a keep alive to verify the STA is still associated. If the STA does ACK
     * the data-null, or if the data-null is buffered and the STA does not
     * retrieve it, the STA will be considered unresponsive (see
     * WMI_VDEV_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS). */
    WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_IDLE_INACTIVE_TIME_SECS,

    /* An associated STA is considered unresponsive if there is no recent
     * TX/RX activity and downlink frames are buffered for it. Once a STA
     * exceeds the maximum unresponsive time, the AP will send a
     * WMI_STA_KICKOUT event to the host so the STA can be deleted. */
    WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS, 

    /* Enable NAWDS : MCAST INSPECT Enable, NAWDS Flag set */
    WMI_VDEV_PARAM_AP_ENABLE_NAWDS, 
    /** Enable/Disable RTS-CTS */
    WMI_VDEV_PARAM_ENABLE_RTSCTS,
    /* Enable TXBFee/er */
    WMI_VDEV_PARAM_TXBF,

    /**Set packet power save */
    WMI_VDEV_PARAM_PACKET_POWERSAVE,
} WMI_VDEV_PARAM;

        /** slot time long */
        #define WMI_VDEV_SLOT_TIME_LONG                                  0x1 
        /** slot time short */
        #define WMI_VDEV_SLOT_TIME_SHORT                                 0x2 
        /** preablbe long */
        #define WMI_VDEV_PREAMBLE_LONG                                   0x1 
        /** preablbe short */
        #define WMI_VDEV_PREAMBLE_SHORT                                  0x2 

/** the definition of different START/RESTART Event response  */
    typedef enum {
        /* Event respose of START CMD */
        WMI_VDEV_START_RESP_EVENT = 0,
        /* Event respose of RESTART CMD */
        WMI_VDEV_RESTART_RESP_EVENT,
    } WMI_START_EVENT_PARAM;
        
        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** requestor id that requested the VDEV start request */
            A_UINT32 requestor_id;
            /* Respose of Event type START/RESTART */
            WMI_START_EVENT_PARAM resp_type;
            /** status of the response */
            A_UINT32 status;   
        } wmi_vdev_start_response_event;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
        } wmi_vdev_standby_req_event;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
        } wmi_vdev_resume_req_event;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
        } wmi_vdev_stopped_event;

        /** common structure used for simple events (stopped, resume_req, standby response) */
        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
        } wmi_vdev_simple_event;


        /** VDEV start response status codes */
        #define WMI_VDEV_START_RESPONSE_STATUS_SUCCESS 0x0  /** VDEV succesfully started */
        #define WMI_VDEV_START_RESPONSE_INVALID_VDEVID  0x1  /** requested VDEV not found */
        #define WMI_VDEV_START_RESPONSE_NOT_SUPPORTED  0x2  /** unsupported VDEV combination */

        /** Beacon processing related command and event structures */
        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** xmit rate */
            A_UINT32 tx_rate;
            /** xmit power */
            A_UINT32 txPower;
            /** beacon buffer length in bytes */
            A_UINT32 buf_len;
        } wmi_bcn_tx_hdr;

        typedef struct {
            /** header */
            wmi_bcn_tx_hdr hdr;
            /** beacon buffer */
            A_UINT8 bufp[1];
        } wmi_bcn_tx_cmd;

        /* Beacon filter */
        #define WMI_BCN_FILTER_ALL   0 /* Filter all beacons */
        #define WMI_BCN_FILTER_NONE  1 /* Pass all beacons */
        #define WMI_BCN_FILTER_RSSI  2 /* Pass Beacons RSSI >= RSSI threshold */
        #define WMI_BCN_FILTER_BSSID 3 /* Pass Beacons with matching BSSID */
        #define WMI_BCN_FILTER_SSID  4 /* Pass Beacons with matching SSID */

        typedef struct {
            /** Filter ID */
            A_UINT32 bcn_filter_id;
            /** Filter type - wmi_bcn_filter */
            A_UINT32 bcn_filter;
            /** Buffer len */
            A_UINT32 bcn_filter_len;
            /** Filter info (threshold, BSSID, RSSI) */
            A_UINT8 *bcn_filter_buf;
        } wmi_bcn_filter_rx_cmd;

        /** Capabilities and IEs to be passed to firmware */
        typedef struct {
            /** Capabilities */
            A_UINT32 caps;
            /** ERP info */
            A_UINT32 erp;
            /** Advanced capabilities */
            /** HT capabilities */
            /** HT Info */
            /** ibss_dfs */
            /** wpa Info */
            /** rsn Info */
            /** rrm info */
            /** ath_ext */
            /** app IE */
        } wmi_bcn_prb_info;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** TIM IE offset from the beginning of the template. */
            A_UINT32 tim_ie_offset;
            /** beacon probe capabilities and IEs */
            wmi_bcn_prb_info bcn_prb_info;
            /** beacon buffer length */
            A_UINT32 buf_len;
            /** Variable length data */
            A_UINT8  data[1];
        } wmi_bcn_tmpl_cmd;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** beacon probe capabilities and IEs */
            wmi_bcn_prb_info bcn_prb_info;
            /** beacon buffer length */
            A_UINT32 buf_len;
            /** Variable length data */
            A_UINT8  data[1];
        } wmi_prb_tmpl_cmd;
        enum wmi_sta_ps_mode {
            /** enable power save for the given STA VDEV */
            WMI_STA_PS_MODE_DISABLED = 0,
            /** disable power save  for a given STA VDEV */
            WMI_STA_PS_MODE_ENABLED = 1,
        };

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;

            /** Power save mode 
             *
             * (see enum wmi_sta_ps_mode)
             */
            A_UINT32 sta_ps_mode;
        } wmi_sta_powersave_mode_cmd;

       enum wmi_csa_offload_en{
           WMI_CSA_OFFLOAD_DISABLE = 0,
           WMI_CSA_OFFLOAD_ENABLE = 1,
       };

       typedef struct{
          A_UINT32 vdev_id;
          A_UINT32 csa_offload_enable;
       } wmi_csa_offload_enable_cmd;

       typedef struct{
          A_UINT32 vdev_id;
          wmi_channel chan;
       } wmi_csa_offload_chanswitch_cmd;
        /** 
         * This parameter controls the policy for retrieving frames from AP while the
         * STA is in sleep state.  
         *
         * Only takes affect if the sta_ps_mode is enabled 
         */
        enum wmi_sta_ps_param_rx_wake_policy {
            /* Wake up when ever there is an  RX activity on the VDEV. In this mode
             * the Power save SM(state machine) will come out of sleep by either
             * sending null frame (or) a data frame (with PS==0) in response to TIM
             * bit set in the received beacon frame from AP.
             */
            WMI_STA_PS_RX_WAKE_POLICY_WAKE = 0,

            /* Here the power save state machine will not wakeup in response to TIM
             * bit, instead it will send a PSPOLL (or) UASPD trigger based on UAPSD
             * configuration setup by WMISET_PS_SET_UAPSD  WMI command.  When all
             * access categories are delivery-enabled, the station will send a UAPSD
             * trigger frame, otherwise it will send a PS-Poll.
             */
            WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD = 1,
        };

        /** Number of tx frames/beacon  that cause the power save SM to wake up.
         *
         * Value 1 causes the SM to wake up for every TX. Value 0 has a special
         * meaning, It will cause the SM to never wake up. This is useful if you want
         * to keep the system to sleep all the time for some kind of test mode . host
         * can change this parameter any time.  It will affect at the next tx frame.
         */
        enum wmi_sta_ps_param_tx_wake_threshold {
            WMI_STA_PS_TX_WAKE_THRESHOLD_NEVER = 0,
            WMI_STA_PS_TX_WAKE_THRESHOLD_ALWAYS = 1,

            /* Values greater than one indicate that many TX attempts per beacon
             * interval before the STA will wake up
             */
        };

        /**
         * The maximum number of PS-Poll frames the FW will send in response to
         * traffic advertised in TIM before waking up (by sending a null frame with PS
         * = 0). Value 0 has a special meaning: there is no maximum count and the FW
         * will send as many PS-Poll as are necessary to retrieve buffered BU. This
         * parameter is used when the RX wake policy is
         * WMI_STA_PS_RX_WAKE_POLICY_POLL_UAPSD and ignored when the RX wake
         * policy is WMI_STA_PS_RX_WAKE_POLICY_WAKE.
         */
        enum wmi_sta_ps_param_pspoll_count {
            WMI_STA_PS_PSPOLL_COUNT_NO_MAX = 0,
            /* Values greater than 0 indicate the maximum numer of PS-Poll frames FW
             * will send before waking up.
             */
        };

        /*
         * This will include the delivery and trigger enabled state for every AC.
         * This is the negotiated state with AP. The host MLME needs to set this based
         * on AP capability and the state Set in the association request by the
         * station MLME.Lower 8 bits of the value specify the UAPSD configuration.
         */
        #define WMI_UAPSD_AC_TYPE_DELI 0
        #define WMI_UAPSD_AC_TYPE_TRIG 1

        #define WMI_UAPSD_AC_BIT_MASK(ac,type) (type ==  WMI_UAPSD_AC_TYPE_DELI)?(1<<(ac<<1)):(1<<((ac<<1)+1)) 

        enum wmi_sta_ps_param_uapsd {
            WMI_STA_PS_UAPSD_AC0_DELIVERY_EN = (1 << 0),
            WMI_STA_PS_UAPSD_AC0_TRIGGER_EN  = (1 << 1),
            WMI_STA_PS_UAPSD_AC1_DELIVERY_EN = (1 << 2),
            WMI_STA_PS_UAPSD_AC1_TRIGGER_EN  = (1 << 3),
            WMI_STA_PS_UAPSD_AC2_DELIVERY_EN = (1 << 4),
            WMI_STA_PS_UAPSD_AC2_TRIGGER_EN  = (1 << 5),
            WMI_STA_PS_UAPSD_AC3_DELIVERY_EN = (1 << 6),
            WMI_STA_PS_UAPSD_AC3_TRIGGER_EN  = (1 << 7),
        };

        enum wmi_sta_powersave_param {
            /** 
             * Controls how frames are retrievd from AP while STA is sleeping
             *
             * (see enum wmi_sta_ps_param_rx_wake_policy) 
             */
            WMI_STA_PS_PARAM_RX_WAKE_POLICY = 0,

            /** 
             * The STA will go active after this many TX
             *
             * (see enum wmi_sta_ps_param_tx_wake_threshold)
             */
            WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD = 1,

            /** 
             * Number of PS-Poll to send before STA wakes up
             *
             * (see enum wmi_sta_ps_param_pspoll_count)
             *
             */
            WMI_STA_PS_PARAM_PSPOLL_COUNT = 2,

            /** 
             * TX/RX inactivity time in msec before going to sleep.
             *
             * The power save SM will monitor tx/rx activity on the VDEV, if no
             * activity for the specified msec of the parameter the Power save SM will
             * go to sleep.
             */
            WMI_STA_PS_PARAM_INACTIVITY_TIME = 3,

            /**
             * Set uapsd configuration. 
             *
             * (see enum wmi_sta_ps_param_uapsd)
             */
            WMI_STA_PS_PARAM_UAPSD = 4,
        };

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** station power save parameter (see enum wmi_sta_powersave_param) */
            A_UINT32 param;
            A_UINT32 value;
        } wmi_sta_powersave_param_cmd;

         /** No MIMO power save */
        #define WMI_STA_MIMO_PS_MODE_DISABLE      
         /** mimo powersave mode static*/    
        #define WMI_STA_MIMO_PS_MODE_STATIC
         /** mimo powersave mode dynamic */    
        #define WMI_STA_MIMO_PS_MODE_DYNAMI

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** mimo powersave mode as defined above */
            A_UINT32 mimo_pwrsave_mode;
        } wmi_sta_mimo_ps_mode_cmd;


        /** U-APSD configuration of peer station from (re)assoc request and TSPECs */
        enum wmi_ap_ps_param_uapsd {
            WMI_AP_PS_UAPSD_AC0_DELIVERY_EN = (1 << 0),
            WMI_AP_PS_UAPSD_AC0_TRIGGER_EN  = (1 << 1),
            WMI_AP_PS_UAPSD_AC1_DELIVERY_EN = (1 << 2),
            WMI_AP_PS_UAPSD_AC1_TRIGGER_EN  = (1 << 3),
            WMI_AP_PS_UAPSD_AC2_DELIVERY_EN = (1 << 4),
            WMI_AP_PS_UAPSD_AC2_TRIGGER_EN  = (1 << 5),
            WMI_AP_PS_UAPSD_AC3_DELIVERY_EN = (1 << 6),
            WMI_AP_PS_UAPSD_AC3_TRIGGER_EN  = (1 << 7),
        };

        /** U-APSD maximum service period of peer station */
        enum wmi_ap_ps_peer_param_max_sp {
            WMI_AP_PS_PEER_PARAM_MAX_SP_UNLIMITED = 0,
            WMI_AP_PS_PEER_PARAM_MAX_SP_2 = 1,
            WMI_AP_PS_PEER_PARAM_MAX_SP_4 = 2,
            WMI_AP_PS_PEER_PARAM_MAX_SP_6 = 3,

            /* keep last! */
            MAX_WMI_AP_PS_PEER_PARAM_MAX_SP,
        };

        /** 
         * AP power save parameter 
         * Set a power save specific parameter for a peer station 
         */
        enum wmi_ap_ps_peer_param {
            /** Set uapsd configuration for a given peer. 
             *
             * This will include the delivery and trigger enabled state for every AC.
             * The host  MLME needs to set this based on AP capability and stations
             * request Set in the association request  received from the station. 
             *
             * Lower 8 bits of the value specify the UAPSD configuration.  
             *
             * (see enum wmi_ap_ps_param_uapsd)
             * The default value is 0.
             */
            WMI_AP_PS_PEER_PARAM_UAPSD = 0,

            /** 
             * Set the service period for a UAPSD capable station 
             *
             * The service period from wme ie in the (re)assoc request frame. 
             *
             * (see enum wmi_ap_ps_peer_param_max_sp)
             */
            WMI_AP_PS_PEER_PARAM_MAX_SP = 1,

            /** Time in seconds for aging out buffered frames for STA in power save */
            WMI_AP_PS_PEER_PARAM_AGEOUT_TIME = 2,
        };

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** peer MAC address */
            wmi_mac_addr peer_macaddr;
            /** AP powersave param (see enum wmi_ap_ps_peer_param) */
            A_UINT32 param;
            /** AP powersave param value */
            A_UINT32 value;
        } wmi_ap_ps_peer_cmd;

        /** Configure peer station 11v U-APSD coexistance 
         *
         * Two parameters from uaspd coexistence ie info (as specified in 11v) are
         * sent down to FW along with this command.
         *
         * The semantics of these fields are described in the following text extracted
         * from 802.11v.
         *
         * ---  If the non-AP STA specified a non-zero TSF 0 Offset value in the
         *      U-APSD Coexistence element, the AP should not transmit frames to the
         *      non-AP STA outside of the U-APSD Coexistence Service Period, which
         *      begins when the AP receives the U-APSD trigger frame and ends after
         *      the transmission period specified by the result of the following
         *      calculation:
         *
         *          End of transmission period = T + (Interval . ((T . TSF 0 Offset) mod Interval))
         *
         *      Where T is the time the U-APSD trigger frame was received at the AP 
         *      Interval is the UAPSD Coexistence element Duration/Interval field
         *      value (see 7.3.2.91) or upon the successful transmission of a frame
         *      with EOSP bit set to 1, whichever is earlier.
         *
         *
         * ---  If the non-AP STA specified a zero TSF 0 Offset value in the U-APSD
         *      Coexistence element, the AP should not transmit frames to the non-AP
         *      STA outside of the U-APSD Coexistence Service Period, which begins
         *      when the AP receives a U-APSD trigger frame and ends after the
         *      transmission period specified by the result of the following
         *      calculation: End of transmission period = T + Duration
         */
        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** peer MAC address */
            wmi_mac_addr peer_macaddr;
            /** Enable U-APSD coexistence support for this peer 
             *
             * 0 -> disabled (default)
             * 1 -> enabled 
             */
            A_UINT32 enabled;
            /** Duration/Interval as defined by 11v U-ASPD coexistance */
            A_UINT32 duration_interval;
            /** Upper 32 bits of 64-bit TSF offset */
            A_UINT32 tsf_offset_high;
            /** Lower 32 bits of 64-bit TSF offset */
            A_UINT32 tsf_offset_low;
        } wmi_ap_powersave_peer_uapsd_coex_cmd;

        /* 128 clients = 4 words */
        #define WMI_TIM_BITMAP_ARRAY_SIZE 4

        typedef struct {
            /** TIM bitmap len (in bytes)*/
            A_UINT32 tim_len;
            /** TIM Partial Virtual Bitmap */
            A_UINT32 tim_mcast;
            A_UINT32 tim_bitmap[WMI_TIM_BITMAP_ARRAY_SIZE];
            A_UINT32 tim_changed;
            A_UINT32 tim_num_ps_pending;
        } wmi_tim_info;

        typedef struct {
            /** Flag to enable quiet period IE support */
            A_UINT32   is_enabled;
            /** Quiet start */
            A_UINT32   tbttcount;
            /** Beacon intervals between quiets*/
            A_UINT32   period;
            /** TUs of each quiet*/
            A_UINT32   duration;
            /** TUs of from TBTT of quiet start*/
            A_UINT32   offset;
        } wmi_quiet_info;

        typedef struct {
            /** TIM info */
            wmi_tim_info tim_info;
            /* TBD: More info elements to be added later */
        } wmi_bcn_info;

        typedef struct {
            /** bitmap identifying the VDEVs, generated by the caller */
            A_UINT32 vdev_map;
            /** bcn info for each VDEV set in the vdev_map arranged as a list */
            wmi_bcn_info bcn_info[1];
        } wmi_host_swba_event;

        #define WMI_MAX_AP_VDEV 16


        typedef struct {
            /** bimtap of VDEVs that has tbtt offset updated */
            A_UINT32 vdev_map;
            /** tbtt offset list in the order of the LSB to MSB in the vdev_map bitmap */
            A_UINT32 tbttoffset_list[WMI_MAX_AP_VDEV];
        } wmi_tbtt_offset_event;


        /* Peer Specific commands and events */
        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** peer MAC address */
            wmi_mac_addr peer_macaddr;
        } wmi_peer_create_cmd;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** peer MAC address */
            wmi_mac_addr peer_macaddr;
        } wmi_peer_delete_cmd;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** peer MAC address */
            wmi_mac_addr peer_macaddr;
            /** tid bitmap identifying the tids to flush */
            A_UINT32 peer_tid_bitmap;
        } wmi_peer_flush_tids_cmd;

        typedef struct {
            /** rate mode . 0: disable fixed rate (auto rate)
             *   1: legacy (non 11n) rate  specified as ieee rate 2*Mbps  
             *   2: ht20 11n rate  specified as mcs index  
             *   3: ht40 11n rate  specified as mcs index  
             */
            A_UINT32  rate_mode;
             /** 4 rate values for 4 rate series. series 0 is stored in byte 0 (LSB) 
              *  and series 3 is stored at byte 3 (MSB) */
            A_UINT32  rate_series;
             /** 4 retry counts for 4 rate series. retry count for rate 0 is stored in byte 0 (LSB) 
              *  and retry count for rate 3 is stored at byte 3 (MSB) */
            A_UINT32  rate_retries;
        } wmi_fixed_rate;

        typedef struct {
            /** unique id identifying the VDEV, generated by the caller */
            A_UINT32 vdev_id;
            /** peer MAC address */
            wmi_mac_addr peer_macaddr;
            /** fixed rate */
            wmi_fixed_rate peer_fixed_rate;
        } wmi_peer_fixed_rate_cmd;

        #define WMI_MGMT_TID    17

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
} wmi_addba_clear_resp_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** Buffer/Window size*/
    A_UINT32 buffersize;
} wmi_addba_send_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** Is Initiator */
    A_UINT32 initiator;
    /** Reason code */
    A_UINT32 reasoncode;
} wmi_delba_send_cmd;

typedef struct {
    /** unique id identifying the vdev, generated by the caller */
    A_UINT32 vdev_id;
    /** peer mac address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** status code */
    A_UINT32 statuscode;
} wmi_addba_setresponse_cmd;

typedef struct {
    /** unique id identifying the vdev, generated by the caller */
    A_UINT32 vdev_id;
    /** peer mac address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
} wmi_send_singleamsdu_cmd;

/** mimo powersave state */
#define WMI_PEER_MIMO_PS_STATE                          0x1
/** enable/disable AMPDU . initial value (enabled) */
#define WMI_PEER_AMPDU                                  0x2
/** authorize/unauthorize peer. initial value is unauthorized (0)  */
#define WMI_PEER_AUTHORIZE                              0x3
/** peer channel bandwidth */
#define WMI_PEER_CHWIDTH                                0x4
/** peer NSS */
#define WMI_PEER_NSS                                    0x5
/** USE 4 ADDR */
#define WMI_PEER_USE_4ADDR                              0x6
/* set group membership status */
#define WMI_PEER_MEMBERSHIP                             0x7
#define WMI_PEER_USERPOS                                0x8

/** mimo ps values for the parameter WMI_PEER_MIMO_PS_STATE  */                        
#define WMI_PEER_MIMO_PS_NONE                          0x0
#define WMI_PEER_MIMO_PS_STATIC                        0x1
#define WMI_PEER_MIMO_PS_DYNAMIC                       0x2

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** parameter id   */
    A_UINT32 param_id;
    /** parametr value */
    A_UINT32 param_value;
} wmi_peer_set_param_cmd;

#define MAX_SUPPORTED_RATES 128 

typedef struct {
    /** total number of rates */  
    A_UINT32 num_rates;                     
    /**
     * rates (each 8bit value) packed into a 32 bit word.
     * the rates are filled from least significant byte to most
     * significant byte. 
     */
    A_UINT32 rates[(MAX_SUPPORTED_RATES/4)+1]; 
} wmi_rate_set;

/* NOTE: It would bea good idea to represent the Tx MCS
 * info in one word and Rx in another word. This is split
 * into multiple words for convenience
 */
typedef struct {
    A_UINT32 rx_max_rate; /* Max Rx data rate */
    A_UINT32 rx_mcs_set;  /* Negotiated RX VHT rates */
    A_UINT32 tx_max_rate; /* Max Tx data rate */
    A_UINT32 tx_mcs_set;  /* Negotiated TX VHT rates */
}wmi_vht_rate_set;

typedef struct {
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** legacy rate set */
    wmi_rate_set peer_legacy_rates;
    /** ht rate set */
    wmi_rate_set peer_ht_rates;
}   wmi_peer_set_rates_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    A_UINT32     callback_enable;
} wmi_peer_set_q_empty_callback_cmd;
/*
 * IMPORTANT: Make sure the bit definitions here are consistent
 * with the ni_flags definitions in wlan_peer.h
 */
#define WMI_PEER_AUTH           0x00000001  /* Authorized for data */
#define WMI_PEER_QOS            0x00000002  /* QoS enabled */
#define WMI_PEER_NEED_PTK_4_WAY 0x00000004  /* Needs PTK 4 way handshake for authorization */
#define WMI_PEER_NEED_GTK_2_WAY 0x00000010  /* Needs GTK 2 way handshake after 4-way handshake */
#define WMI_PEER_APSD           0x00000800  /* U-APSD power save enabled */
#define WMI_PEER_HT             0x00001000  /* HT enabled */
#define WMI_PEER_40MHZ          0x00002000  /* 40MHz enabld */
#define WMI_PEER_STBC           0x00008000  /* STBC Enabled */
#define WMI_PEER_LDPC           0x00010000  /* LDPC ENabled */
#define WMI_PEER_DYN_MIMOPS     0x00020000  /* Dynamic MIMO PS Enabled */
#define WMI_PEER_STATIC_MIMOPS  0x00040000  /* Static MIMO PS enabled */
#define WMI_PEER_SPATIAL_MUX    0x00200000  /* SM Enabled */
#define WMI_PEER_VHT            0x02000000  /* VHT Enabled */
#define WMI_PEER_80MHZ          0x04000000  /* 80MHz enabld */
#define WMI_PEER_PMF            0x08000000  /* Robust Management Frame Protection enabled */
/** TODO: Place holder for WLAN_PEER_F_PS_PRESEND_REQUIRED = 0x10000000. Need to be clean up */

/**
 * Peer rate capabilities.
 *
 * This is of interest to the ratecontrol
 * module which resides in the firmware. The bit definitions are
 * consistent with that defined in if_athrate.c.
 *
 * @todo
 * Move this to a common header file later so there is no need to
 * duplicate the definitions or maintain consistency.
 */
#define WMI_RC_DS_FLAG          0x01    /* Dual stream flag */
#define WMI_RC_CW40_FLAG        0x02    /* CW 40 */
#define WMI_RC_SGI_FLAG         0x04    /* Short Guard Interval */
#define WMI_RC_HT_FLAG          0x08    /* HT */
#define WMI_RC_RTSCTS_FLAG      0x10    /* RTS-CTS */
#define WMI_RC_TX_STBC_FLAG     0x20    /* TX STBC */
#define WMI_RC_RX_STBC_FLAG     0xC0    /* RX STBC ,2 bits */
#define WMI_RC_RX_STBC_FLAG_S   6       /* RX STBC ,2 bits */
#define WMI_RC_WEP_TKIP_FLAG    0x100   /* WEP/TKIP encryption */
#define WMI_RC_TS_FLAG          0x200   /* Three stream flag */
#define WMI_RC_UAPSD_FLAG       0x400   /* UAPSD Rate Control */

typedef struct {
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** VDEV id */
    A_UINT32 vdev_id;
    /** assoc = 1 reassoc = 0 */
    A_UINT32 peer_new_assoc;
    /** peer associd (16 bits) */
    A_UINT32 peer_associd;
    /** peer station flags: see definition above */
    A_UINT32 peer_flags;
    /** negotiated capabilities (lower 16 bits)*/
    A_UINT32 peer_caps;
    /** Listen interval */
    A_UINT32 peer_listen_intval;
    /** HT capabilties of the peer */
    A_UINT32 peer_ht_caps;
    /** maximum rx A-MPDU length */
    A_UINT32 peer_max_mpdu;
    /** mpdu density of the peer in usec(0 to 16) */
    A_UINT32 peer_mpdu_density;
    /** peer rate capabilties see flags above */
    A_UINT32 peer_rate_caps;
    /** negotiated legacy rate set */
    wmi_rate_set peer_legacy_rates;
    /** negotiated ht rate set */
    wmi_rate_set peer_ht_rates;
    /** num spatial streams */
    A_UINT32 peer_nss;
    /** VHT capabilties of the peer */
    A_UINT32 peer_vht_caps;
    /** phy mode */
    A_UINT32 peer_phymode;
    /** VHT capabilties of the peer */
    wmi_vht_rate_set peer_vht_rates;
    /** HT Operation Element of the peer. Five bytes packed in 2
     *  INT32 array and filled from lsb to msb. */
    A_UINT32 peer_ht_info[2];
} wmi_peer_assoc_complete_cmd;

typedef struct {
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** wds MAC addr */
    wmi_mac_addr wds_macaddr;
} wmi_peer_add_wds_entry_cmd;

typedef struct {
    /** wds MAC addr */
    wmi_mac_addr wds_macaddr;
} wmi_peer_remove_wds_entry_cmd;
 

typedef struct {
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
} wmi_peer_q_empty_callback_event;



/**
 * Channel info WMI event 
 */
typedef struct {
     /** Error code */
    A_UINT32 err_code;
   /** Channel freq */
    A_UINT32 freq; 
    /** Read flags */
    A_UINT32 cmd_flags;
    /** Noise Floor value */
    A_UINT32 noise_floor;
    /** rx clear count */
    A_UINT32   rx_clear_count;
    /** cycle count */
    A_UINT32   cycle_count;
} wmi_chan_info_event; 

/** 
 * Non wlan interference event
 */
typedef struct {
    A_UINT32 channel; /* either number or freq in mhz*/
} ath_dcs_cw_int; 
/** WLAN interference type, either generated from host itself, or from the
 * traget. 
 */
typedef struct {
    A_UINT32    tsf64_hi;               /* time stamp when this event is generated*/
    A_UINT32    tsf64_lo;
    A_UINT32    bytecntrx;              /* received data counter */
    A_UINT32    rxtime;                 /* total time for which received valid packets */
    A_UINT32    wastedtxtime;           /* unsuccessful transmission time */ 
    A_UINT32    bytecnttx;              /* transmitted data counter */
    A_UINT32    phyerrcnt;              /* phy error counter */ 
    /* can add one counter for RTS failures too */
} ath_dcs_wlan_int_stat;

typedef struct {
    A_UINT32    interference_type;      /* type of interference, wlan or cw */
    union {
        ath_dcs_cw_int          cw_int;
        ath_dcs_wlan_int_stat   wlan_stat;
    } int_event;
 } wmi_dcs_interference_event_t;

enum wmi_peer_mcast_group_action {
    wmi_peer_mcast_group_action_add = 0,
    wmi_peer_mcast_group_action_del = 1
};
#define WMI_PEER_MCAST_GROUP_FLAG_ACTION_M   0x1
#define WMI_PEER_MCAST_GROUP_FLAG_ACTION_S   0
#define WMI_PEER_MCAST_GROUP_FLAG_WILDCARD_M 0x2
#define WMI_PEER_MCAST_GROUP_FLAG_WILDCARD_S 1
/* multicast group membership commands */
typedef union {
    struct {
        A_UINT8 flags;
        A_UINT8 pad1;
        A_UINT8 ucast_mac_addr[6]; /* in network byte order */
        A_UINT8 mcast_ip_addr[16]; /* in network byte order */
    } fields;
    A_UINT32 dummy_align[6];
} wmi_peer_mcast_group_cmd;

/** Offload Scan and Roaming related  commands */
/** The FW performs 2 different kinds of offload scans independent 
 *  of host. One is Roam scan which is primarily performed  on a 
 *  station VDEV after association to look for a better AP that 
 *  the station VDEV can roam to. The second scan is connect scan 
 *  which is mainly performed when the station is not associated 
 *  and to look for a matching AP profile from a list of 
 *  configured profiles. */

/** 
 * WMI_ROAM_SCAN_MODE: Set Roam Scan mode 
 *   the roam scan mode is one of the periodic, rssi change, both, none.  
 *   None        : Disable Roam scan. No Roam scan at all.
 *   Periodic    : Scan periodically with a configurable period.
 *   Rssi change : Scan when ever rssi to current AP changes by the threshold value 
 *                 set by WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD command.
 *   Both        : Both of the above (scan when either period expires or rss to current AP changes by X amount)
 *  
 */
typedef struct {
	A_UINT32 roam_scan_mode;
} wmi_roam_scan_mode;

#define WMI_ROAM_SCAN_MODE_NONE        0x0
#define WMI_ROAM_SCAN_MODE_PERIODIC    0x1
#define WMI_ROAM_SCAN_MODE_RSSI_CHANGE 0x2
#define WMI_ROAM_SCAN_MODE_BOTH        0x3

/** 
 * WMI_ROAM_SCAN_RSSI_THRESHOLD : set scan rssi thresold
 *  scan rssi threshold is the rssi threshold below which the FW will start running Roam scans. 
 * Applicable when WMI_ROAM_SCAN_MODE is not set to none.
 */
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** roam scan rssi threshold */
    A_UINT32 roam_scan_rssi_thresh;
} wmi_roam_scan_rssi_threshold;

/**
 * WMI_ROAM_SCAN_PERIOD: period for roam scan.
 *  Applicable when the scan mode is Periodic or both.
 */
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** roam scan period value */
    A_UINT32 roam_scan_period;
} wmi_roam_scan_period;

/**
 * WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD : rssi delta to trigger the roam scan.
 *   Rssi change threshold used when mode is Rssi change (or) Both. 
 *   The FW will run the roam scan when ever the rssi changes (up or down) by the value set by this parameter.
 *   Note scan is triggered based on the rssi threshold condition set by WMI_ROAM_SCAN_RSSI_THRESHOLD
 */
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** roam scan rssi change threshold value */
    A_UINT32 roam_scan_rssi_change_thresh;
} wmi_roam_scan_rssi_change_threshold;


/** Authentication modes */
enum {
    WMI_AUTH_NONE   , /* no upper level auth */
    WMI_AUTH_OPEN   , /* open */
    WMI_AUTH_SHARED , /* shared-key */
    WMI_AUTH_8021X  , /* 802.1x */
    WMI_AUTH_AUTO   , /* Auto */
    WMI_AUTH_WPA    , /* WPA */
    WMI_AUTH_RSNA   , /* WPA2/RSNA */
    WMI_AUTH_CCKM   , /* CCK */
    WMI_AUTH_WAPI   ,/* WAPI */
};

typedef struct {
    /** authentication mode (defined above)  */
    A_UINT32               rsn_authmode;
    /** unicast cipher set */
    A_UINT32               rsn_ucastcipherset;
    /** mcast/group cipher set */
    A_UINT32               rsn_mcastcipherset;
    /** mcast/group management frames cipher set */
    A_UINT32               rsn_mcastmgmtcipherset;
} wmi_rsn_params;

/** looking for a wps enabled AP */
#define WMI_AP_PROFILE_FLAG_WPS    0x1  
/** looking for a secure AP  */  
#define WMI_AP_PROFILE_FLAG_CRYPTO 0x2 

/** To match an open AP, the rs_authmode should be set to WMI_AUTH_NONE   
 *  and WMI_AP_PROFILE_FLAG_CRYPTO should be clear.
 *  To match a WEP enabled AP, the rs_authmode should be set to WMI_AUTH_NONE   
 *  and WMI_AP_PROFILE_FLAG_CRYPTO should be set .
 */   

typedef struct {
    /** flags as defined above */
    A_UINT32  flags;
	/**
	 * rssi thresold value: the value of the the candidate AP should 
	 * higher by this threshold than the rssi of the currrently associated AP.
	 */
	A_UINT32 rssi_threshold;
	/**
	 * ssid vlaue to be matched.
	 */
    wmi_ssid ssid;

	/** 
	 * security params to be matched.
	 */
     wmi_rsn_params rsn_params;	
} wmi_ap_profile;

/** Beacon filter wmi command info */

#define BCN_FLT_MAX_SUPPORTED_IES    256
#define BCN_FLT_MAX_ELEMS_IE_LIST    BCN_FLT_MAX_SUPPORTED_IES/32
    
typedef struct bss_bcn_stats {
    A_UINT32    vdev_id;
    A_UINT32    bss_bcnsdropped;
    A_UINT32    bss_bcnsdelivered;
}wmi_bss_bcn_stats_t;

typedef struct bcn_filter_stats {
    A_UINT32    bcns_dropped;
    A_UINT32    bcns_delivered;
    A_UINT32    activefilters;
    wmi_bss_bcn_stats_t bss_stats;
}wmi_bcnfilter_stats_t;

typedef struct wmi_add_bcn_filter_cmd {
   A_UINT32    vdev_id;
   A_UINT32    ie_map[BCN_FLT_MAX_ELEMS_IE_LIST];
}wmi_add_bcn_filter_cmd_t;

typedef struct wmi_rmv_bcn_filter_cmd {
    A_UINT32    vdev_id;
}wmi_rmv_bcn_filter_cmd_t;

#define WMI_BCN_SEND_DTIM_ZERO         1
#define WMI_BCN_SEND_DTIM_BITCTL_SET   2
typedef struct wmi_bcn_send_from_host {
    A_UINT32 vdev_id;
    A_UINT32 data_len;
    A_UINT32 frag_ptr; /* Physical address of the frame */
    A_UINT32 frame_ctrl; /* farme ctrl to setup PPDU desc */
    A_UINT32 dtim_flag;   /* to control CABQ traffic */
}wmi_bcn_send_from_host_cmd_t;

/* cmd to support bcn snd for all vaps at once */
typedef struct wmi_pdev_send_bcn {
    A_UINT32                       num_vdevs;
    wmi_bcn_send_from_host_cmd_t   bcn_cmd[1];
} wmi_pdev_send_bcn_cmd_t;

 /*
  * WMI_ROAM_AP_PROFILE:  AP profile of connected AP for roaming.
  */
typedef struct {
	/** id of AP criteria */ 
	A_UINT32 id;

    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;

    /** AP profile info */
    wmi_ap_profile ap_profile;

} wmi_roam_ap_profile;

/**
 * WMI_OFL_SCAN_ADD_AP_PROFILE: add an AP profile.
 */
typedef struct {
	/** id of AP criteria */ 
	A_UINT32 id;

    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;

    /** AP profile info */
    wmi_ap_profile ap_profile;

} wmi_ofl_scan_add_ap_profile;

/**
 * WMI_OFL_SCAN_REMOVE_AP_CRITERIA: remove an ap profile.
 */
typedef struct {
	/** id of AP criteria */ 
	A_UINT32 id;
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
} wmi_ofl_scan_remove_ap_profile;

/**
 * WMI_OFL_SCAN_PERIOD: period in msec for offload scan.
 *  0 will disable ofload scan and a very low value will perform a continous
 *  scan.
 */
typedef struct {
	/** offload scan period value, used for scans used when not connected */
	A_UINT32 ofl_scan_period;
} wmi_ofl_scan_period;



/** WMI_ROAM_EVENT: roam event triggering the host roam logic.
 * generated when ever a better AP is found in the recent roam scan (or)
 * when beacon miss is detected (or) when a DEAUTH/DISASSOC is received 
 * from the current AP.
 */
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
	/** reason for roam event */ 
	A_UINT32 reason;

} wmi_roam_event;

#define WMI_ROAM_REASON_BETTER_AP 0x1 /** found a better AP */
#define WMI_ROAM_REASON_BMISS     0x2 /** beacon miss detected */
#define WMI_ROAM_REASON_DEAUTH    0x2 /** deauth/disassoc received */  

/** WMI_PROFILE_MATCH_EVENT: offload scan 
 * generated when ever atleast one of the matching profiles is found 
 * in recent NLO scan. no data is carried with the event. 
 */

/** P2P specific commands */

/** 
 * WMI_P2P_DEV_SET_DEVICE_INFO : p2p device info, which will be used by 
 * FW to generate P2P IE tobe carried in probe response frames.
 * FW will respond to probe requests while in listen state.
 */
typedef struct {
    /* number of secondary device types,supported */
    A_UINT32 num_secondary_dev_types;
    /**
     * followed by 8 bytes of primary device id and
     * num_secondary_dev_types * 8 bytes of secondary device
     * id.
     */
} wmi_p2p_dev_set_device_info;

/** WMI_P2P_DEV_SET_DISCOVERABILITY: enable/disable discoverability
 *  state. if enabled, an active STA/AP will respond to P2P probe requests on
 *  the operating channel of the VDEV.
 */

typedef struct {
    /* 1:enable disoverability, 0:disable discoverability */
    A_UINT32 enable_discoverability;
}  wmi_p2p_set_discoverability;

/** WMI_P2P_GO_SET_BEACON_IE: P2P IE to be added to
 *  beacons generated by FW. used in FW beacon mode.
 *  the FW will add this IE to beacon in addition to the beacon 
 *  template set by WMI_BCN_TMPL_CMDID command.
 */ 
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /* ie length */
    A_UINT32 ie_buf_len;
    /*followed by  byte stream of ie data of length ie_buf_len */ 
}  wmi_p2p_go_set_beacon_ie;

/** WMI_P2P_GO_PROBE_RESP_IE: P2P IE to be added to
 *  probe response generated by FW. used in FW beacon mode.
 *  the FW will add this IE to probe response in addition to the probe response 
 *  template set by WMI_PRB_TMPL_CMDID command.
 */ 
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /* ie length */
    A_UINT32 ie_buf_len;
    /*followed by  byte stream of ie data of length ie_buf_len */ 
}  wmi_p2p_go_set_probe_resp_ie;

/*----RTT Report event definition  ----*/
typedef struct {
    A_UINT32 req_id;                   //identify the command
    /*result is a bit mask
     *bit 0: Success 1 / Fail 0
     *bit 1: Measurement Finished 1 / Measurement not Finished 0
     *bit 15:2 Reserved for future extension
     *bit 16 result Successfully or not 0-fail, 1-success
     *bit 17 0 -- Measurement has not finished 1 -- Measurement has finished
     *bit 31:18 Reserved
     */
    wmi_mac_addr  dest_mac;            //used for future differentiate STAs in one req
}wmi_rtt_event_hdr;

typedef enum {
    RTT_COMMAND_HEADER_ERROR = 0,      //rtt cmd header parsing error  --terminate
    RTT_COMMAND_ERROR,                 //rtt body parsing error -- skip current STA REQ
    RTT_MODULE_BUSY,                   //rtt no resource        -- terminate
    RTT_TOO_MANY_STA,                  //STA exceed the support limit -- only server the first n STA
    RTT_NO_RESOURCE,                   // any allocate failure
    RTT_VDEV_ERROR,                    //can not find vdev with vdev ID -- skip current STA REQ
    RTT_TRANSIMISSION_ERROR,           //Tx failure   -- continiue and measure number--
    RTT_TM_TIMER_EXPIRE,               //wait for first TM timer expire   -- terminate current STA measurement
    RTT_FRAME_TYPE_NOSUPPORT,          //we do not support RTT measurement with this type of frame
    RTT_TIMER_EXPIRE,                  //whole RTT measurement timer expire  -- terminate current STA measurement
    RTT_CHAN_SWITCH_ERROR,            //channel swicth failed
    WMI_RTT_REJECT_MAX,	      
} WMI_RTT_ERROR_INDICATOR;

typedef struct {
    A_UINT32 time32;     //upper 32 bits of time stamp
    A_UINT32 time0;      //lower 32 bits of time stamp
} A_TIME64;

typedef struct {
    wmi_rtt_event_hdr header;
    A_TIME64 toa;                     // resolution of 10ns
    A_TIME64 tod;                     // resolution of 10ns
    A_UINT32 rx_chain;                // Rx chain info
  /************************************************************
   * Bit:0-3:chain mask                                       *
   * Bit 4-5: band width info                                 *
   * 00 --Legacy 20, 01 --HT/VHT20                            *
   * 10 --HT/VHT40, 11 -- VHT80                               *
   ************************************************************/
    //chain report body will be expand here
    //including rssi + channel dump for each chain
}wmi_rtt_meas_event;

typedef struct {
    wmi_rtt_event_hdr header;
    WMI_RTT_ERROR_INDICATOR reject_reason;
}wmi_rtt_error_report_event;

typedef struct {
    A_UINT32 req_id;                   //identify the command
    A_UINT32 result;                   //Successfully or not 0-fail, 1-success
    A_UINT64 peer_tsf;                 //TSF of peer
    A_UINT64 self_tsf;                 //Self's TSF
    A_UINT32 beacon_delta;             //time delta between peer and self's beacon
}wmi_rtt_tsf_meas_event;
/*---- end of RTT report event definition ----*/

typedef struct {
    /** peer mac address */
    wmi_mac_addr peer_macaddr;
} wmi_peer_sta_kickout_event;

#define WMI_WLAN_PROFILE_MAX_HIST     3
#define WMI_WLAN_PROFILE_MAX_BIN_CNT 32

typedef struct _wmi_wlan_profile_t {
    A_UINT32 id;
    A_UINT32 cnt;
    A_UINT32 tot;
    A_UINT32 min;
    A_UINT32 max;
    A_UINT32 hist_intvl;
    A_UINT32 hist[WMI_WLAN_PROFILE_MAX_HIST];
} wmi_wlan_profile_t;
 
typedef struct _wmi_wlan_profile_ctx_t {
    A_UINT32 tot; /* time in us */
    A_UINT32 tx_msdu_cnt;
    A_UINT32 tx_mpdu_cnt;
    A_UINT32 tx_ppdu_cnt;
    A_UINT32 rx_msdu_cnt;
    A_UINT32 rx_mpdu_cnt;
    A_UINT32 bin_count;
} wmi_wlan_profile_ctx_t;

/** Host-based rate-control related commands  */
typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32     vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    A_UINT32     psdu_len    [NUM_DYN_BW * NUM_SCHED_ENTRIES];
    A_UINT32     flags       [NUM_DYN_BW * NUM_SCHED_ENTRIES];
    A_UINT32     rix         [NUM_DYN_BW * NUM_SCHED_ENTRIES];
    A_UINT32     tpc         [NUM_DYN_BW * NUM_SCHED_ENTRIES];
    A_UINT32     num_mpdus   [NUM_DYN_BW * NUM_SCHED_ENTRIES];
    A_UINT32     antmask     [NUM_SCHED_ENTRIES];
    A_UINT32     txbf_cv_ptr;
    A_UINT32     txbf_cv_len;
    A_UINT32     tries       [NUM_SCHED_ENTRIES];
    A_UINT32     num_valid_rates;
    A_UINT32     paprd_mask;
    A_UINT32     rts_rix;
    A_UINT32     sh_pream;
    A_UINT32     min_spacing_1_4_us;
    A_UINT32     fixed_delims;
    A_UINT32     bw_in_service;
    A_UINT32     probe_rix;
}wmi_peer_rate_retry_sched_cmd;

typedef struct {
    wmi_wlan_profile_ctx_t profile_ctx;
    wmi_wlan_profile_t profile_data[WMI_WLAN_PROFILE_MAX_BIN_CNT];
}wmi_profile_stats_event;

typedef struct {
    A_UINT32 enable;
} wmi_wlan_profile_trigger_cmd;

typedef struct {
    A_UINT32 value;
} wmi_wlan_profile_get_prof_data_cmd;

typedef struct {
    A_UINT32 profile_id;
    A_UINT32 value;
} wmi_wlan_profile_set_hist_intvl_cmd;

typedef struct {
    A_UINT32 profile_id;
    A_UINT32 enable;
} wmi_wlan_profile_enable_profile_id_cmd;

/*Wifi header is upto 26, LLC is 8, with 14 byte duplicate in 802.3 header, that's 26+8-14=20.
146-128=18. So this means it is converted to non-QoS header. Riva FW take care of the QOS/non-QOS
when comparing wifi header.*/
#define WOW_DEFAULT_BITMAP_PATTERN_SIZE      146
#define WOW_DEFAULT_BITMASK_SIZE             146
#define WOW_MAX_BITMAP_FILTERS               22
#define WOW_DEFAULT_MAGIG_PATTERN_MATCH_CNT  16
#define WOW_DEFAULT_EVT_BUF_SIZE             128  /* Maximum 128 bytes of the data is copied starting from header
                                                   * incase if the match is found */

typedef enum pattern_type_e {
    WOW_PATTERN_MIN = 0,
    WOW_BITMAP_PATTERN = WOW_PATTERN_MIN,
    WOW_IPV4_SYNC_PATTERN,
    WOW_IPV6_SYNC_PATTERN,
    WOW_WILD_CARD_PATTERN,
    WOW_TIMER_PATTERN,
    WOW_PATTERN_MAX
}WOW_PATTERN_TYPE;

typedef enum event_type_e {
    WOW_BMISS_EVENT = 0,
    WOW_DEAUTH_RECVD_EVENT,
    WOW_MAGIC_PKT_RECVD_EVENT,
    WOW_GTK_ERR_EVENT,
    WOW_GTK_UPDATE_EVENT,
    WOW_FOURWAY_HSHAKE_EVENT,
    WOW_EAPOL_RECVD_EVENT
}WOW_WAKE_EVENT_TYPE;

typedef enum wake_reason_e {
    WOW_REASON_NLOD = 0,
    WOW_REASON_AP_ASSOC_LOST,
    WOW_REASON_DEAUTH_RECVD,
    WOW_REASON_DISASSOC_RECVD,
    WOW_REASON_GTK_HS_ERR,
    WOW_REASON_FOURWAY_HS_RECV,
    WOW_REASON_TIMER_INTR_RECV,
    WOW_REASON_PATTERN_MATCH_FOUND,
    WOW_REASON_RECV_MAGIC_PATTERN,
}WOW_WAKE_REASON_TYPE;

typedef struct bitmap_pattern_s {
    A_UINT8      patternbuf[WOW_DEFAULT_BITMAP_PATTERN_SIZE];
    A_UINT8      bitmaskbuf[WOW_DEFAULT_BITMASK_SIZE];
    A_UINT8      pattern_offset;
    A_UINT32     pattern_len;
    A_UINT32     bitmask_len;
    A_UINT32     pattern_id;  /* must be less than max_bitmap_filters */
}WOW_BITMAP_PATTERN_T;

typedef struct ipv4_sync_s {
    A_UINT8      ipv4_src_addr[4];
    A_UINT8      ipv4_dst_addr[4];
    A_UINT32     tcp_src_prt;
    A_UINT32     tcp_dst_prt;
}WOW_IPV4_SYNC_PATTERN_T;

typedef struct ipv6_sync_s {
    A_UINT8      ipv6_src_addr[16];
    A_UINT8      ipv6_dst_addr[16];
    A_UINT32     tcp_src_prt;
    A_UINT32     tcp_dst_prt;
}WOW_IPV6_SYNC_PATTERN_T;



typedef struct {
    A_UINT32        pattern_id;
    A_UINT32        pattern_type;
    union {
        WOW_BITMAP_PATTERN_T       bitmap;
        WOW_IPV4_SYNC_PATTERN_T    ipv4;
        WOW_IPV6_SYNC_PATTERN_T    ipv6;
        A_UINT32                   timeout;
    }pattern_info;
}WMI_WOW_ADD_PATTERN_CMD;

typedef struct {
    A_UINT32        pattern_id;
    A_UINT32        pattern_type;
}WMI_WOW_DEL_PATTERN_CMD;

typedef struct {
    A_UINT32    is_add;
    A_UINT32    event_bitmap;
}WMI_WOW_ADD_DEL_EVT_CMD;

typedef struct  event_info_s {
    A_UINT32    wake_reason;
    A_UINT32    data_len;
}EVENT_INFO;

#define WMI_RXERR_CRC               0x01    /* CRC error on frame */
#define WMI_RXERR_DECRYPT           0x08    /* non-Michael decrypt error */
#define WMI_RXERR_MIC               0x10    /* Michael MIC decrypt error */
#define WMI_RXERR_KEY_CACHE_MISS    0x20    /* No/incorrect key matter in h/w */

typedef enum {
    PKT_PWR_SAVE_PAID_MATCH =         0x0001,
    PKT_PWR_SAVE_GID_MATCH =          0x0002,
    PKT_PWR_SAVE_EARLY_TIM_CLEAR =    0x0004,
    PKT_PWR_SAVE_EARLY_DTIM_CLEAR =   0x0008,
    PKT_PWR_SAVE_EOF_PAD_DELIM =      0x0010,
    PKT_PWR_SAVE_MACADDR_MISMATCH =   0x0020,
    PKT_PWR_SAVE_DELIM_CRC_FAIL =     0x0040,
    PKT_PWR_SAVE_GID_NSTS_ZERO =      0x0080,
    PKT_PWR_SAVE_RSSI_CHECK =         0x0100,
    WMI_PKT_PWR_SAVE_MAX =            0x0200,
} WMI_PKT_PWR_SAVE_TYPE;

typedef struct {
    /* This structure is used to send Factory Test Mode [FTM] command
     * to firmware for integrated chips which are binary blobs.
     * data array variable is used to access binary blob.
     * One byte is to used insure this structure compiles on all platforms
     */
    A_UINT8 data[1];
}wmi_ftm_intg_cmd;

typedef struct {
    /* This structure is used to receive Factory Test Mode [FTM] event
     * from firmware to host for integrated chips which are binary blobs.
     */
    A_UINT8 data[1];
}wmi_ftm_intg_event;

#define WMI_MAX_NS_OFFLOADS           2
#define WMI_MAX_ARP_OFFLOADS          2

#define WMI_ARPOFF_FLAGS_VALID              (1 << 0)    /* the tuple entry is valid */
#define WMI_ARPOFF_FLAGS_MAC_VALID          (1 << 1)    /* the target mac address is valid */
#define WMI_ARPOFF_FLAGS_REMOTE_IP_VALID    (1 << 2)    /* remote IP field is valid */

typedef struct _WMI_IPV6_ADDR {
    A_UINT8    address[16];    /* IPV6 in Network Byte Order */
} WMI_IPV6_ADDR;


typedef struct {
    A_UINT32           flags;                   /* flags */
    A_UINT8           target_ipaddr[4];        /* IPV4 addresses of the local node*/
    A_UINT8           remote_ipaddr[4];        /* source address of the remote node requesting the ARP (qualifier) */
    A_UINT8           target_mac[ATH_MAC_LEN]; /* mac address for this tuple, if not valid, the local MAC is used */
} WMI_ARP_OFFLOAD_TUPLE;

#define WMI_NSOFF_FLAGS_VALID           (1 << 0)    /* the tuple entry is valid */
#define WMI_NSOFF_FLAGS_MAC_VALID       (1 << 1)    /* the target mac address is valid */
#define WMI_NSOFF_FLAGS_REMOTE_IP_VALID (1 << 2)    /* remote IP field is valid */

#define WMI_NSOFF_MAX_TARGET_IPS    2

typedef struct {
    A_UINT32           flags;                     /* flags */
    WMI_IPV6_ADDR     target_ipaddr[WMI_NSOFF_MAX_TARGET_IPS]; /* IPV6 target addresses of the local node  */
    WMI_IPV6_ADDR     solicitation_ipaddr;       /* multi-cast source IP addresses for receiving solicitations */
    WMI_IPV6_ADDR     remote_ipaddr;             /* address of remote node requesting the solicitation (qualifier) */
    A_UINT8           target_mac[ATH_MAC_LEN];   /* mac address for this tuple, if not valid, the local MAC is used */
} WMI_NS_OFFLOAD_TUPLE;

typedef struct {
    A_UINT32                flags;
    WMI_NS_OFFLOAD_TUPLE    ns_tuples[WMI_MAX_NS_OFFLOADS];
    WMI_ARP_OFFLOAD_TUPLE   arp_tuples[WMI_MAX_ARP_OFFLOADS];
} WMI_SET_ARP_NS_OFFLOAD_CMD;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** Initiator (1) or Responder (0) for this aggregation */
    A_UINT32 initiator;
    /** size of the negotiated window */
    A_UINT32 window_size;
    /** starting sequence number (only valid for initiator) */
    A_UINT32 ssn;
    /** timeout field represents the time to wait for Block Ack in
    *   initiator case and the time to wait for BAR in responder
    *   case. 0 represents no timeout. */
    A_UINT32 timeout;
    /* BA policy: immediate ACK (0) or delayed ACK (1) */
    A_UINT32 policy;
} wmi_peer_tid_addba_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** Initiator (1) or Responder (0) for this aggregation */
    A_UINT32 initiator;
} wmi_peer_tid_delba_cmd;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** Event status */
    A_UINT32 status;
} wmi_tx_addba_complete_event;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** peer MAC address */
    wmi_mac_addr peer_macaddr;
    /** Tid number */
    A_UINT32 tid;
    /** Event status */
    A_UINT32 status;
} wmi_tx_delba_complete_event;

typedef struct {
    /** unique id identifying the VDEV, generated by the caller */
    A_UINT32 vdev_id;
    /** MAC address used for installing   */
    wmi_mac_addr peer_macaddr;
    /** key index */
    A_UINT32 key_ix;
    /** key flags */
    A_UINT32 key_flags;
    /** Event status */
    A_UINT32 status;
} wmi_vdev_install_key_complete_event;

#define WMI_NLO_MAX_SSIDS    16
#define WMI_NLO_MAX_CHAN     48

#define WMI_NLO_CONFIG_STOP      (0x1 << 0)
#define WMI_NLO_CONFIG_START     (0x1 << 1)
#define WMI_NLO_CONFIG_RESET     (0x1 << 2)
#define WMI_NLO_CONFIG_SLOW_SCAN (0x1 << 4)


typedef struct wmi_nlo_config {
    A_UINT32    flags;
    A_UINT32    vdev_id;
    A_UINT32    max_scan_cycles;
    A_UINT32    active_dwell_time;
    A_UINT32    passive_dwell_time; /* PDT in msecs */
    A_UINT32    probe_bundle_size;
    A_UINT32    rest_time;  /* ART = IRT */
    A_UINT32    max_rest_time; /* Max value that can be reached after SBM */
    A_UINT32    scan_backoff_multiplier;  /* SBM */
    A_UINT32    fast_scan_period; /* SCBM */
    A_UINT32    slow_scan_period; /* specific to windows */
    A_UINT32    no_of_ssids;
    wmi_ssid    ssid_list[WMI_NLO_MAX_SSIDS];
    A_UINT32    num_of_channels;
    A_UINT32    channel_list[WMI_NLO_MAX_CHAN];
} wmi_nlo_config_cmd;

#define GTK_OFFLOAD_OPCODE_MASK				0xFF000000
/** Enable GTK offload, and provided parameters KEK,KCK and replay counter values */  
#define GTK_OFFLOAD_ENABLE_OPCODE			0x01000000
/** Disable GTK offload */	
#define GTK_OFFLOAD_DISABLE_OPCODE			0x02000000
/** Read GTK offload parameters, generates WMI_GTK_OFFLOAD_STATUS_EVENT */	
#define GTK_OFFLOAD_REQUEST_STATUS_OPCODE	0x04000000
enum wmi_chatter_mode {
    /* Chatter enter/exit happens
     * automatically based on preset
     * params
     */
    WMI_CHATTER_MODE_AUTO,
    /* Chatter enter is triggered
     * manually by the user
     */
    WMI_CHATTER_MODE_MANUAL_ENTER,
    /* Chatter exit is triggered
     * manually by the user
     */
    WMI_CHATTER_MODE_MANUAL_EXIT,
    /* Placeholder max value, always last*/
    WMI_CHATTER_MODE_MAX
};

typedef struct {
    A_UINT32 chatter_mode;
} wmi_chatter_set_mode_cmd;

#define GTK_OFFLOAD_KEK_BYTES       16
#define GTK_OFFLOAD_KCK_BYTES       16
#define GTK_REPLAY_COUNTER_BYTES    8

typedef struct {
    A_UINT32 	vdev_id;    					/** unique id identifying the VDEV */
    A_UINT32    flags;          				/* status flags */
    A_UINT32    refresh_cnt;    				/* number of successful GTK refresh exchanges since last SET operation */     
    A_UINT8     replay_counter[GTK_REPLAY_COUNTER_BYTES]; /* current replay counter */
} WMI_GTK_OFFLOAD_STATUS_EVENT;

typedef struct {
    A_UINT32 	vdev_id;    					/** unique id identifying the VDEV */
    A_UINT32    flags;                          /* control flags, GTK offload command use high byte  */
    A_UINT8     KEK[GTK_OFFLOAD_KEK_BYTES];     /* key encryption key */
    A_UINT8     KCK[GTK_OFFLOAD_KCK_BYTES];     /* key confirmation key */
    A_UINT8     replay_counter[GTK_REPLAY_COUNTER_BYTES];  /* replay counter for re-key */
}WMI_GTK_OFFLOAD_CMD;

typedef struct {
    A_UINT32    vdev_id;
    A_UINT32 keepaliveInterval;   /* seconds */
} wmi_vdev_set_keepalive_cmd;

typedef struct {
    A_UINT32    vdev_id;
} wmi_vdev_get_keepalive_cmd;

typedef struct {
    A_UINT32    vdev_id;
    A_UINT32 keepaliveInterval;   /* seconds */
} wmi_vdev_get_keepalive_event;

#ifdef __cplusplus
}
#endif

#endif /*_WMI_UNIFIED_H_*/

/**@}*/
