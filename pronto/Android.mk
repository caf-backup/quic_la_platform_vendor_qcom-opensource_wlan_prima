# Android makefile for the WLAN WCNSS/Pronto Module

LOCAL_PATH := $(call my-dir)
WLAN_BLD_DIR := vendor/qcom/proprietary/wlan
ifeq ($(call is-android-codename,JELLY_BEAN),true)
       DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
       DLKM_DIR := build/dlkm
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := WCNSS_qcom_wlan_nv.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist
LOCAL_SRC_FILES    := firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := WCNSS_cfg.dat
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan/prima
LOCAL_SRC_FILES    := firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := WCNSS_qcom_cfg.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist
LOCAL_SRC_FILES    := firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# Build pronto_wlan.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_PRONTO=../$(WLAN_BLD_DIR)/pronto
KBUILD_OPTIONS += PRIMA_SRC_DIR=../$(WLAN_BLD_DIR)/prima
KBUILD_OPTIONS += WCNSS_DIR=../vendor/qcom/proprietary/wlan/wcnss
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

include $(CLEAR_VARS)
LOCAL_MODULE              := pronto_wlan.ko
LOCAL_MODULE_KBUILD_NAME  := wlan.ko
LOCAL_MODULE_TAGS         := debug
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(TARGET_OUT)/lib/modules/pronto
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

#Create symbolic link
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
        ln -sf /system/lib/modules/pronto/pronto_wlan.ko \
               $(TARGET_OUT)/lib/modules/wlan.ko)

$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wlan/prima; \
        ln -sf /persist/WCNSS_qcom_wlan_nv.bin \
        $(TARGET_OUT_ETC)/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin; \
        ln -sf /data/misc/wifi/WCNSS_qcom_cfg.ini \
        $(TARGET_OUT_ETC)/firmware/wlan/prima/WCNSS_qcom_cfg.ini)
