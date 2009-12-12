/******************************************************************************
*
* Name:  btcApi.h
*
* Description: BTC Events Layer API definitions.
*
* Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
* Qualcomm Confidential and Proprietary.
*
******************************************************************************/

#ifndef __BTC_API_H__
#define __BTC_API_H__

#include "vos_types.h"

#define BT_INVALID_CONN_HANDLE (0xFFFF)  /**< Invalid connection handle */

/* ACL and Sync connection attempt results */
#define BT_CONN_STATUS_FAIL      (0)         /**< Connection failed */
#define BT_CONN_STATUS_SUCCESS   (1)         /**< Connection successful */
#define	BT_CONN_STATUS_MAX		 (2)		 /**< This and beyond are invalid values */

/** ACL and Sync link types
  These must match the Bluetooth Spec!
*/
#define BT_SCO					(0)   /**< SCO Link */
#define BT_ACL					(1)   /**< ACL Link */
#define BT_eSCO					(2)   /**< eSCO Link */
#define BT_LINK_TYPE_MAX		(3)   /**< This value and higher are invalid */

/**	ACL link modes
	These must match the Bluetooth Spec!
*/
#define BT_ACL_ACTIVE			(0)   /**< Active mode */
#define BT_ACL_HOLD				(1)   /**< Hold mode */
#define BT_ACL_SNIFF			(2)   /**< Sniff mode */
#define BT_ACL_PARK				(3)   /**< Park mode */
#define BT_ACL_MODE_MAX	    	(4)   /**< This value and higher are invalid */

/**	BTC Executions Modes allowed to be set by user
*/
#define BTC_SMART_COEXISTENCE	(0)	/** BTC Mapping Layer decides whats best */
#define	BTC_WLAN_ONLY			(1) /** WLAN takes all mode */
#define	BTC_PTA_ONLY            (2) /** Allow only 3 wire protocol in H/W */
#define	BT_EXEC_MODE_MAX		(3) /** This and beyond are invalid values */

/**	Enumeration of different kinds actions that BTC Mapping Layer
	can do if PM indication (to AP) fails.
*/
#define	BTC_RESTART_CURRENT		(0) /** Restart the interval we just failed to leave */
#define	BTC_START_NEXT          (1) /** Start the next interval even though the PM transition at the AP was unsuccessful */
#define	BTC_ACTION_TYPE_MAX     (2) /** This and beyond are invalid values */

/** Bitmaps used for maintaining various BT events that requires
    enough time to complete such that it might require disbling of
    heartbeat monitoring to avoid WLAN link loss with the AP
*/
#define BT_INQUIRY_STARTED                  (1<<0)
#define BT_PAGE_STARTED                     (1<<1)
#define BT_CREATE_ACL_CONNECTION_STARTED    (1<<2)
#define BT_CREATE_SYNC_CONNECTION_STARTED   (1<<3)

/** Maximum time duration in milliseconds between a specific BT start event and its
	respective stop event, before it can be declared timed out on receiving the stop event.
*/
#define BT_MAX_EVENT_DONE_TIMEOUT	15000

/**	Enumeration of all the different kinds of BT events
*/
typedef enum eSmeBtEventType
{
  BT_EVENT_DEVICE_SWITCHED_ON = 0,
  BT_EVENT_DEVICE_SWITCHED_OFF,
  BT_EVENT_INQUIRY_STARTED,
  BT_EVENT_INQUIRY_STOPPED,
  BT_EVENT_INQUIRY_SCAN_STARTED,
  BT_EVENT_INQUIRY_SCAN_STOPPED,
  BT_EVENT_PAGE_STARTED,
  BT_EVENT_PAGE_STOPPED,
  BT_EVENT_PAGE_SCAN_STARTED,
  BT_EVENT_PAGE_SCAN_STOPPED,
  BT_EVENT_CREATE_ACL_CONNECTION,
  BT_EVENT_ACL_CONNECTION_COMPLETE,
  BT_EVENT_CREATE_SYNC_CONNECTION,
  BT_EVENT_SYNC_CONNECTION_COMPLETE,
  BT_EVENT_SYNC_CONNECTION_UPDATED,
  BT_EVENT_DISCONNECTION_COMPLETE,
  BT_EVENT_MODE_CHANGED,
  BT_EVENT_A2DP_STREAM_START,
  BT_EVENT_A2DP_STREAM_STOP,
  BT_EVENT_TYPE_MAX,    //This and beyond are invalid values
} tSmeBtEventType;

/**Data structure that specifies the needed event parameters for
	BT_EVENT_CREATE_ACL_CONNECTION and BT_EVENT_ACL_CONNECTION_COMPLETE
*/
typedef struct sSmeBtAclConnectionParam
{
   v_U8_t		bdAddr[6];
   v_U16_t		connectionHandle;
   v_U8_t		status;
} tSmeBtAclConnectionParam, *tpSmeBtAclConnectionParam;

/**	Data structure that specifies the needed event parameters for
	BT_EVENT_CREATE_SYNC_CONNECTION, BT_EVENT_SYNC_CONNECTION_COMPLETE
	and BT_EVENT_SYNC_CONNECTION_UPDATED
*/
typedef struct sSmeBtSyncConnectionParam
{
   v_U8_t		bdAddr[6];
   v_U16_t		connectionHandle;
   v_U8_t		status;
   v_U8_t		linkType;
   v_U8_t		scoInterval; //units in number of 625us slots
   v_U8_t		scoWindow;   //units in number of 625us slots
   v_U8_t		retransmisisonWindow; //units in number of 625us slots
} tSmeBtSyncConnectionParam, *tpSmeBtSyncConnectionParam;

/**Data structure that specifies the needed event parameters for
	BT_EVENT_MODE_CHANGED
*/
typedef struct sSmeBtAclModeChangeParam
{
	v_U16_t		connectionHandle;
	v_U8_t		mode;
} tSmeBtAclModeChangeParam, *tpSmeBtAclModeChangeParam;

/*Data structure that specifies the needed event parameters for
	BT_EVENT_DISCONNECTION_COMPLETE
*/
typedef struct sSmeBtDisconnectParam
{
   v_U16_t connectionHandle;
} tSmeBtDisconnectParam, *tpSmeBtDisconnectParam;

/**	Generic Bluetooth Event structure for BTC
*/
typedef struct sSmeBtcBtEvent
{
   tSmeBtEventType btEventType;
   union
   {
      v_U8_t					bdAddr[6];    /**< For events with only a BT Addr in event_data */
	  tSmeBtAclConnectionParam	btAclConnection;
      tSmeBtSyncConnectionParam	btSyncConnection;
      tSmeBtDisconnectParam		btDisconnect;
	  tSmeBtAclModeChangeParam	btAclModeChange;
   }uEventParam;
} tSmeBtEvent, *tpSmeBtEvent;

/**	Data structure that specifies the BTC Configuration parameters
*/
typedef struct sSmeBtcConfig
{
   v_U8_t		btcExecutionMode;
   v_U8_t		btcActionOnPmFail;
   v_U8_t		btcBtIntervalMode1;
   v_U8_t		btcWlanIntervalMode1;
   v_U8_t       btcEventState;

   v_U8_t       btcHBActive;    /* Is HB currently active */
   v_U8_t		btcHBCount;     /* default HB count */
   vos_timer_t	restoreHBTimer;	/* Timer to restore heart beat */

} tSmeBtcConfig, *tpSmeBtcConfig;


/**Place holder for all BTC related information
*/
typedef struct sSmeBtcInfo
{
   tSmeBtcConfig btcConfig;
   v_BOOL_t btcReady;
} tSmeBtcInfo, *tpSmeBtcInfo;


/**	Routine definitions
*/
VOS_STATUS btcOpen (tHalHandle hHal);
VOS_STATUS btcClose (tHalHandle hHal);
VOS_STATUS btcReady (tHalHandle hHal);
VOS_STATUS btcSendCfgMsg(tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig);
VOS_STATUS btcSignalBTEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent);
VOS_STATUS btcSetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig);
VOS_STATUS btcGetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig);

#endif
