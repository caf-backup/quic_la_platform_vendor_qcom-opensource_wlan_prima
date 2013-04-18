/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#ifndef __ALLJOYN_DBUS_H
#define __ALLJOYN_DBUS_H

#ifdef BLUETOOTH_BLUEDROID

#include "abtfilt_int.h"

/* bluetooth related data structures from BlueZ */
/* BD Address */
typedef struct {
	uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

/* Baseband links */
#define SCO_LINK	0x00
#define ACL_LINK	0x01
#define ESCO_LINK	0x02

#define HCI_MAX_EVENT_SIZE	260

/* HCI Packet types */
#define HCI_COMMAND_PKT		0x01
#define HCI_ACLDATA_PKT		0x02
#define HCI_SCODATA_PKT		0x03
#define HCI_EVENT_PKT		0x04
#define HCI_VENDOR_PKT		0xff

/* --------  HCI Packet structures  -------- */
typedef struct {
	uint16_t	opcode;		/* OCF & OGF */
	uint8_t		plen;
} __attribute__ ((packed))	hci_command_hdr;
#define HCI_COMMAND_HDR_SIZE	3

typedef struct {
	uint8_t		evt;
	uint8_t		plen;
} __attribute__ ((packed))	hci_event_hdr;
#define HCI_EVENT_HDR_SIZE	2


/* Command opcode pack/unpack */
#define cmd_opcode_pack(ogf, ocf)	(uint16_t)((ocf & 0x03ff)|(ogf << 10))
#define cmd_opcode_ogf(op)		(op >> 10)
#define cmd_opcode_ocf(op)		(op & 0x03ff)

/* -----  HCI Commands ----- */
/* Link Control */
#define OGF_LINK_CTL		0x01
#define OCF_INQUIRY_CANCEL		0x0002

#define OCF_INQUIRY			0x0001
#define OCF_PERIODIC_INQUIRY		0x0003
#define OCF_CREATE_CONN			0x0005
#define OCF_READ_REMOTE_FEATURES	0x001B
#define OCF_READ_REMOTE_VERSION		0x001D

/* Link Policy */
#define OGF_LINK_POLICY         0x02
#define OCF_ROLE_DISCOVERY              0x0009

/* Informational Parameters */
#define OGF_INFO_PARAM		0x04

#define OCF_READ_BD_ADDR		0x0009

/* ---- HCI Events ---- */
#define EVT_INQUIRY_COMPLETE		0x01
#define EVT_CONN_COMPLETE		0x03
#define EVT_CONN_REQUEST		0x04
#define EVT_CMD_COMPLETE		0x0E
typedef struct {
	uint8_t		ncmd;
	uint16_t	opcode;
} __attribute__ ((packed)) evt_cmd_complete;

#define EVT_READ_REMOTE_FEATURES_COMPLETE	0x0B
#define EVT_READ_REMOTE_VERSION_COMPLETE	0x0C
#define EVT_PIN_CODE_REQ		0x16
#define EVT_LINK_KEY_NOTIFY		0x18

#define EVT_SYNC_CONN_COMPLETE		0x2C
typedef struct {
	uint8_t		status;
	uint16_t	handle;
	bdaddr_t	bdaddr;
	uint8_t		link_type;
	uint8_t		trans_interval;
	uint8_t		retrans_window;
	uint16_t	rx_pkt_len;
	uint16_t	tx_pkt_len;
	uint8_t		air_mode;
} __attribute__ ((packed)) evt_sync_conn_complete;



/* Byte order conversions */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobs(d)  (d)
#define htobl(d)  (d)
#define htobll(d) (d)
#define btohs(d)  (d)
#define btohl(d)  (d)
#define btohll(d) (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htobs(d)  bswap_16(d)
#define htobl(d)  bswap_32(d)
#define htobll(d) bswap_64(d)
#define btohs(d)  bswap_16(d)
#define btohl(d)  bswap_32(d)
#define btohll(d) bswap_64(d)
#else
#error "Unknown byte order"
#endif


/*
        for Bluedroid
  */
#define STACK_SOCKET_NAME "qcom.btc.server"
#define HCI_SOCKET_NAME "btc_hci"

enum BTCEvent {
	BD_NONE = 0x00,
	BD_BT_ADAPTER_ADDED = 0x20,
	BD_BT_ADAPTER_REMOVED = 0x21,
	BD_DEVICE_DISCOVERY_STARTED = 0x22,
	BD_DEVICE_DISCOVERY_FINISHED = 0x23,
	BD_REMOTE_DEVICE_CONNECTED = 0x24,
	BD_REMOTE_DEVICE_DISCONNECTED = 0x25,
	BD_AUDIO_DEVICE_ADDED = 0x26,
	BD_AUDIO_DEVICE_REMOVED = 0x27,
	BD_AUDIO_HEADSET_CONNECTED = 0x40,
	BD_AUDIO_HEADSET_DISCONNECTED = 0x41,
	BD_AUDIO_HEADSET_STREAM_STARTED = 0x42,
	BD_AUDIO_HEADSET_STREAM_STOPPED = 0x43,
	BD_AUDIO_SINK_CONNECTED = 0x60,
	BD_AUDIO_SINK_DISCONNECTED = 0x61,
	BD_AUDIO_SINK_STREAM_STARTED = 0x62,
	BD_AUDIO_SINK_STREAM_STOPPED = 0x63,
	BD_INPUT_DEVICE_CONNECTED = 0x80,
	BD_INPUT_DEVICE_DISCONNECTED = 0x81,
};

/* modify this definition after adding/deleting any BT event*/
#define BT_EVENTS_NUM_MAX 18

/* abtfilter related data structure */
#define STRING_SIZE_MAX		128
#define BD_ADDR_SIZE		6
#define MAX_SOCKET_RETRY_CNT	10

void Abf_BToff();
typedef void (* BT_EVENT_HANDLER)(void *,void *);

typedef struct _ABF_BT_INFO {
	ATHBT_FILTER_INFO              *pInfo;
	A_BOOL                          AdapterAvailable;
	A_UINT8                         HCIVersion;
	A_UINT16                        HCIRevision;
	A_UINT8                         HCI_LMPVersion;
	A_UINT16                        HCI_LMPSubVersion;
	A_UINT8                         RemoteDevice[BD_ADDR_SIZE];
	A_UINT8                         HCI_DeviceAddress[BD_ADDR_SIZE];
	A_CHAR                          HCI_AdapterName[STRING_SIZE_MAX];
	A_CHAR                          HCI_DeviceName[STRING_SIZE_MAX];
	A_CHAR                          HCI_ManufacturerName[STRING_SIZE_MAX];
	A_CHAR                          HCI_ProtocolVersion[STRING_SIZE_MAX];
	A_BOOL                          AdapterCbRegistered;
	A_CHAR                          DefaultAudioDeviceName[STRING_SIZE_MAX];
	A_CHAR                          DefaultRemoteAudioDeviceAddress[32];
	A_CHAR                          DefaultRemoteAudioDeviceVersion[32];
	A_UINT8                         DefaultAudioDeviceLmpVersion;
	A_BOOL                          DefaultAudioDeviceAvailable;
	A_BOOL                          AudioCbRegistered;
	A_UCHAR                         CurrentSCOLinkType;
	int                             AdapterId;
	int				StackEventSocket; /* socket for BT stack/profile events*/
	A_TASK_HANDLE                   hBtStackEventThread;
	A_BOOL                          StackEventThreadCreated;
	A_BOOL                          StackEventThreadShutdown;
	int                             HCIEventListenerSocket;
	A_TASK_HANDLE                   hBtHCIFilterThread;
	A_BOOL                          HCIFilterThreadCreated;
	A_BOOL                          HCIFilterThreadShutdown;
	BT_EVENT_HANDLER                SignalHandlers[BT_EVENTS_NUM_MAX];
	A_BOOL                          DefaultRemoteAudioDevicePropsValid;
	A_BOOL                          ThreadCreated;
	A_UINT32                        btInquiryState;
} ABF_BT_INFO;

#else /* BLUETOOTH_BLUEDROID */

#include "abtfilt_int.h"
/*-----------------------------------------------------------------------*/
/* BT Section */
#define STRING_SIZE_MAX             128
#define BD_ADDR_SIZE                6

#define BLUEZ_NAME                        "org.bluez"
#define ADAPTER_INTERFACE                 "org.bluez.Adapter"
#define MANAGER_INTERFACE                 "org.bluez.Manager"

#ifdef BLUEZ4_3
#define BLUEZ_PATH                        "/"
#define AUDIO_MANAGER_PATH                "/org/bluez/"
#define AUDIO_MANAGER_INTERFACE           "org.bluez"
#define AUDIO_SINK_INTERFACE              "org.bluez.AudioSink"
#define AUDIO_SOURCE_INTERFACE            "org.bluez.AudioSource"
#define AUDIO_HEADSET_INTERFACE           "org.bluez.Headset"
#define AUDIO_GATEWAY_INTERFACE           "org.bluez.Gateway"
#define AUDIO_DEVICE_INTERFACE            "org.bluez.Device"
#define INPUT_DEVICE_INTERFACE            "org.bluez.Input"
#else
#define BLUEZ_PATH                        "/org/bluez"
#define AUDIO_MANAGER_PATH                "/org/bluez/audio"
#define AUDIO_SINK_INTERFACE              "org.bluez.audio.Sink"
#define AUDIO_SOURCE_INTERFACE            "org.bluez.audio.Source"
#define AUDIO_HEADSET_INTERFACE           "org.bluez.audio.Headset"
#define AUDIO_GATEWAY_INTERFACE           "org.bluez.audio.Gateway"
#define AUDIO_MANAGER_INTERFACE           "org.bluez.audio.Manager"
#define AUDIO_DEVICE_INTERFACE            "org.bluez.audio.Device"
#endif

void Abf_BToff();
typedef void (* BT_EVENT_HANDLER)(void *,void *);

typedef enum {
    BT_ADAPTER_ADDED = 0,
    BT_ADAPTER_REMOVED,
    DEVICE_DISCOVERY_STARTED,
    DEVICE_DISCOVERY_FINISHED,
    REMOTE_DEVICE_CONNECTED,
    REMOTE_DEVICE_DISCONNECTED,
    AUDIO_DEVICE_ADDED,
    AUDIO_DEVICE_REMOVED,
    AUDIO_HEADSET_CONNECTED,
    AUDIO_HEADSET_DISCONNECTED,
    AUDIO_HEADSET_STREAM_STARTED,
    AUDIO_HEADSET_STREAM_STOPPED,
    AUDIO_SINK_CONNECTED,
    AUDIO_SINK_DISCONNECTED,
    AUDIO_SINK_STREAM_STARTED,
    AUDIO_SINK_STREAM_STOPPED,
#ifdef HID_PROFILE_SUPPORT
    INPUT_DEVICE_PROPERTY_CHANGED,
#endif
    BT_EVENTS_NUM_MAX,
} BT_STACK_EVENT;

typedef struct _ABF_BT_INFO {
    ATHBT_FILTER_INFO              *pInfo;
    A_BOOL                          AdapterAvailable;
    A_UINT8                         HCIVersion;
    A_UINT16                        HCIRevision;
    A_UINT8                         HCI_LMPVersion;
    A_UINT16                        HCI_LMPSubVersion;
    A_UINT8                         RemoteDevice[BD_ADDR_SIZE];
    A_UINT8                         HCI_DeviceAddress[BD_ADDR_SIZE];
    A_CHAR                          HCI_AdapterName[STRING_SIZE_MAX];
    A_CHAR                          HCI_DeviceName[STRING_SIZE_MAX];
    A_CHAR                          HCI_ManufacturerName[STRING_SIZE_MAX];
    A_CHAR                          HCI_ProtocolVersion[STRING_SIZE_MAX];
    A_BOOL                          AdapterCbRegistered;
    A_CHAR                          DefaultAudioDeviceName[STRING_SIZE_MAX];
    A_CHAR                          DefaultRemoteAudioDeviceAddress[32];
    A_CHAR                          DefaultRemoteAudioDeviceVersion[32];
    A_UINT8                         DefaultAudioDeviceLmpVersion;
    A_BOOL                          DefaultAudioDeviceAvailable;
    A_BOOL                          AudioCbRegistered;
    A_UCHAR                         CurrentSCOLinkType;
    int                             AdapterId;
    int                             HCIEventListenerSocket;
    A_TASK_HANDLE                   hBtHCIFilterThread;
    A_BOOL                          HCIFilterThreadCreated;
    A_BOOL                          HCIFilterThreadShutdown;
    BT_EVENT_HANDLER                SignalHandlers[BT_EVENTS_NUM_MAX];
    A_BOOL                          DefaultRemoteAudioDevicePropsValid;
    A_BOOL                          ThreadCreated;
    A_UINT32                        btInquiryState;
} ABF_BT_INFO;

#endif /* BLUETOOTH_BLUEDROID */
#endif /*ABTFILT_BTSTACK_DBUS_H_*/
