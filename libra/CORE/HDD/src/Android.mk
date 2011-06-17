# Android makefile for the WLAN Libra Module

# Build/Package only in case of 7x30 and 7x27 target
ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627_ffa msm7630_surf msm7630_fusion msm7627_6x),true)

WLAN_BLD_DIR := vendor/qcom/proprietary/wlan
LIBRA_FW_DIR := vendor/qcom/proprietary/wlan/libra/firmware_bin
DLKM_DIR     := build/dlkm

PRODUCT_COPY_FILES += $(LIBRA_FW_DIR)/qcom_fw.bin:system/etc/firmware/wlan/qcom_fw.bin
PRODUCT_COPY_FILES += $(LIBRA_FW_DIR)/qcom_wapi_fw.bin:system/etc/firmware/wlan/qcom_wapi_fw.bin
PRODUCT_COPY_FILES += $(LIBRA_FW_DIR)/qcom_wlan_nv.bin:persist/qcom_wlan_nv.bin
PRODUCT_COPY_FILES += $(LIBRA_FW_DIR)/cfg.dat:system/etc/firmware/wlan/cfg.dat
PRODUCT_COPY_FILES += $(LIBRA_FW_DIR)/qcom_cfg.ini:data/hostapd/qcom_cfg.ini
PRODUCT_COPY_FILES += $(LIBRA_FW_DIR)/qcom_cfg.ini:persist/qcom/softap/qcom_cfg_default.ini

#Create sym link for ftm driver
WLAN_LIBRA_FTM_SYM_LINK := $(WLAN_BLD_DIR)/libra/ftm/CORE
$(WLAN_LIBRA_FTM_SYM_LINK):
	@mkdir -p $(dir $@)
	ln -sf ../CORE $@

file := $(WLAN_LIBRA_FTM_SYM_LINK)
ALL_PREBUILT += $(file)

# Build libra.ko
###########################################################
LOCAL_PATH := $(call my-dir)

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_LIBRA=../$(WLAN_BLD_DIR)/libra
# We are actually building wlan.ko here, as per the
# requirement we are specifying libra.ko as LOCAL_MODULE.
# This means we need to rename the module to libra.ko after
# wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan

include $(CLEAR_VARS)
LOCAL_MODULE             := libra.ko
LOCAL_MODULE_KBUILD_NAME := wlan.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/libra
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Build libra_ftm.ko
###########################################################
LOCAL_PATH := $(WLAN_BLD_DIR)/libra/ftm/CORE/HDD/src

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := BUILD_FTM_DRIVER=1
KBUILD_OPTIONS += WLAN_LIBRA=../$(WLAN_BLD_DIR)/libra

include $(CLEAR_VARS)
LOCAL_MODULE      := libra_ftm.ko
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules/libra
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

#Create symbolic link
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
WLAN_WCN1312_SYMLINK := $(TARGET_OUT)/lib/modules/wlan.ko
$(WLAN_WCN1312_SYMLINK):
	@mkdir -p $(dir $@)
	ln -sf /system/lib/modules/libra/libra.ko $@

file := $(WLAN_WCN1312_SYMLINK)
ALL_PREBUILT += $(file)
endif
endif
