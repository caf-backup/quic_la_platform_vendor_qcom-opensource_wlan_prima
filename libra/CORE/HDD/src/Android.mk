# Android makefile for the WLAN Libra Module

LOCAL_PATH := $(call my-dir)
WLAN_BLD_DIR := vendor/qcom/proprietary/wlan-noship
DLKM_DIR     := build/dlkm

include $(CLEAR_VARS)
LOCAL_MODULE       := qcom_fw.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := qcom_wapi_fw.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := qcom_wlan_nv.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := cfg.dat
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := qcom_cfg.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/data/hostapd
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := qcom_cfg_default.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist/qcom/softap
LOCAL_SRC_FILES    := ../../../firmware_bin/qcom_cfg.ini
include $(BUILD_PREBUILT)

#Create sym link for ftm driver
$(shell mkdir -p $(WLAN_BLD_DIR)/libra/ftm; \
        ln -sf ../CORE $(WLAN_BLD_DIR)/libra/ftm/CORE)

# Build libra.ko
###########################################################

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_LIBRA=../$(WLAN_BLD_DIR)/libra
# We are actually building wlan.ko here, as per the
# requirement we are specifying libra.ko as LOCAL_MODULE.
# This means we need to rename the module to libra.ko after
# wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

#include $(CLEAR_VARS)
#LOCAL_MODULE             := libra.ko
#LOCAL_MODULE_KBUILD_NAME := wlan.ko
#LOCAL_MODULE_TAGS        := debug
#LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/libra
#include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Build libra_ftm.ko
###########################################################
LOCAL_PATH := $(WLAN_BLD_DIR)/libra/ftm/CORE/HDD/src

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := BUILD_FTM_DRIVER=1
KBUILD_OPTIONS += WLAN_LIBRA=../$(WLAN_BLD_DIR)/libra
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

#include $(CLEAR_VARS)
#LOCAL_MODULE      := libra_ftm.ko
#LOCAL_MODULE_TAGS := debug
#LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules/libra
#include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

#Create symbolic link
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
        ln -sf /system/lib/modules/libra/libra.ko \
               $(TARGET_OUT)/lib/modules/wlan.ko)
endif
