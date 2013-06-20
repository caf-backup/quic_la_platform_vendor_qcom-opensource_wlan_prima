# Android makefile for the WLAN WCN1314 Module

LOCAL_PATH := $(call my-dir)
WLAN_BLD_DIR  := vendor/qcom/proprietary/wlan-noship
ifeq ($(call is-android-codename,JELLY_BEAN),true)
      DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
      DLKM_DIR := build/dlkm
endif

# Default nv.bin to move to persist
FW_NV_FILE := WCN1314_qcom_wlan_nv.bin

#These checks are only for msm7x30 and msm8660 and msm7627a series
#The files mentioned will present in vendor/qcom/proprietary/wlan/volans/firmware_bin/ directory

ifeq ($(call is-board-platform,msm7630_surf),true)
FW_NV_FILE := msm7630_qcom_wlan_nv.bin
endif

ifeq ($(call is-board-platform,msm7627a),true)
FW_NV_FILE := msm7627a_qcom_wlan_nv.bin
endif

ifeq ($(call is-board-platform,msm8660),true)
FW_NV_FILE := msm8660_qcom_wlan_nv.bin
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := WCN1314_qcom_fw.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan/volans
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := $(FW_NV_FILE)
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := WCN1314_cfg.dat
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan/volans
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := WCN1314_qcom_cfg.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan/volans
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# Build WCN1314_rf.ko
###########################################################

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_VOLANS=../$(WLAN_BLD_DIR)/volans
# We are actually building wlan.ko here, as per the
# requirement we are specifying WCN1314_rf.ko as LOCAL_MODULE.
# This means we need to rename the module to WCN1314_rf.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

include $(CLEAR_VARS)
LOCAL_MODULE             := WCN1314_rf.ko
LOCAL_MODULE_KBUILD_NAME := wlan.ko
LOCAL_MODULE_TAGS        := optional
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/volans
include $(DLKM_DIR)/AndroidKernelModule.mk

#Create symbolic link
ifeq ($(call is-board-platform-in-list,msm7627a msm8660 msm7630_surf),true)
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
        ln -sf /system/lib/modules/volans/WCN1314_rf.ko \
               $(TARGET_OUT)/lib/modules/wlan.ko)
endif

$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wlan/volans; \
        ln -sf /persist/$(FW_NV_FILE) \
        $(TARGET_OUT_ETC)/firmware/wlan/volans/WCN1314_qcom_wlan_nv.bin)
