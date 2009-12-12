BUILD_PTT_SOCKET_APP:=1

ifeq ($(BUILD_PTT_SOCKET_APP),1)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
	pttSocketApp.c

LOCAL_CFLAGS += \
	-fno-short-enums 

#LOCAL_CFLAGS += \
	-DANI_DEBUG

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../asf/inc

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils

LOCAL_STATIC_LIBRARIES := \
	libAniAsf

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := ptt_socket_app

include $(BUILD_EXECUTABLE)

endif
