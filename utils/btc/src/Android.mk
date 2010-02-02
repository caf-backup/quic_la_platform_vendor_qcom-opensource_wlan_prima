LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libBtc

LOCAL_SRC_FILES += \
	wlan_btc_usr_svc.c

LOCAL_CFLAGS += \
	-fno-short-enums 

BTC_INCLUDE_PATH := $(LOCAL_PATH)/../inc  
BTC_INCLUDE_PATH += $(LOCAL_PATH)/../../../CORE/SVC/external/

LOCAL_C_INCLUDES:= $(BTC_INCLUDE_PATH)

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
