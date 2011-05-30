# Android makefile for the WLAN WCN1314 Module

# Build/Package only in case of 7x30 and 8660

WLAN_BLD_DIR  := vendor/qcom/proprietary/wlan
VOLANS_FW_DIR := vendor/qcom/proprietary/wlan/volans/firmware_bin
DLKM_DIR      := build/dlkm

PRODUCT_COPY_FILES += $(VOLANS_FW_DIR)/WCN1314_qcom_fw.bin:system/etc/firmware/wlan/volans/WCN1314_qcom_fw.bin
PRODUCT_COPY_FILES += $(VOLANS_FW_DIR)/WCN1314_qcom_wlan_nv.bin:persist/WCN1314_qcom_wlan_nv.bin
PRODUCT_COPY_FILES += $(VOLANS_FW_DIR)/WCN1314_cfg.dat:system/etc/firmware/wlan/volans/WCN1314_cfg.dat
PRODUCT_COPY_FILES += $(VOLANS_FW_DIR)/WCN1314_qcom_cfg.ini:system/etc/firmware/wlan/volans/WCN1314_qcom_cfg.ini

#Create sym link for ftm driver
WLAN_VOLANS_FTM_SYM_LINK := $(WLAN_BLD_DIR)/volans/ftm/CORE
$(WLAN_VOLANS_FTM_SYM_LINK):
	@mkdir -p $(dir $@)
	ln -sf ../CORE $@

file := $(WLAN_VOLANS_FTM_SYM_LINK)
ALL_PREBUILT += $(file)

# Build WCN1314_rf.ko
###########################################################
LOCAL_PATH := $(call my-dir)

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_VOLANS=../$(WLAN_BLD_DIR)/volans
# We are actually building wlan.ko here, as per the
# requirement we are specifying WCN1314_rf.ko as LOCAL_MODULE.
# This means we need to rename the module to WCN1314_rf.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan

include $(CLEAR_VARS)
LOCAL_MODULE             := WCN1314_rf.ko
LOCAL_MODULE_KBUILD_NAME := wlan.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/volans
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Build WCN1314_rf_ftm.ko
###########################################################
LOCAL_PATH := $(WLAN_BLD_DIR)/volans/ftm/CORE/HDD/src

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := BUILD_FTM_DRIVER=1
KBUILD_OPTIONS += WLAN_VOLANS=../$(WLAN_BLD_DIR)/volans

include $(CLEAR_VARS)
LOCAL_MODULE      := WCN1314_rf_ftm.ko
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules/volans
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

#Create symbolic link
ifneq (, $(filter msm7627a msm8660_surf, $(QCOM_TARGET_PRODUCT)))
WLAN_WCN1314_SYMLINK := $(TARGET_OUT)/lib/modules/wlan.ko
$(WLAN_WCN1314_SYMLINK):
	@mkdir -p $(dir $@)
	ln -s -f /system/lib/modules/volans/WCN1314_rf.ko $@

file := $(WLAN_WCN1314_SYMLINK)
ALL_PREBUILT += $(file)
endif

WLAN_NV_FILE_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/volans/WCN1314_qcom_wlan_nv.bin
$(WLAN_NV_FILE_SYMLINK):
	@mkdir -p $(dir $@)
	ln -s -f /persist/WCN1314_qcom_wlan_nv.bin $@

file := $(WLAN_NV_FILE_SYMLINK)
ALL_PREBUILT += $(file)
