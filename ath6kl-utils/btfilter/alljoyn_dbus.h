/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#ifndef __ALLJOYN_DBUS_H
#define __ALLJOYN_DBUS_H

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

#endif /*ABTFILT_BTSTACK_DBUS_H_*/
