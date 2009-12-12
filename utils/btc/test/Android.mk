BUILD_BTC_TEST_APP:=1

ifeq ($(BUILD_BTC_TEST_APP),1)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
	wlan_btc_test_app.c

LOCAL_CFLAGS += \
	-fno-short-enums 

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../inc

LOCAL_SHARED_LIBRARIES := \
	libBtc \
	libutils \
	libcutils

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := wlan_btc_test_app

include $(BUILD_EXECUTABLE)

endif
