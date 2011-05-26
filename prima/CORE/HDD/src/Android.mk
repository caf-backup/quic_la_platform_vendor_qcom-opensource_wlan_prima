# Android makefile for the WLAN WCNSS/Prima Module

# Build/Package only in case of 8960 target

WLAN_BLD_DIR := vendor/qcom/proprietary/wlan
PRIMA_FW_DIR := vendor/qcom/proprietary/wlan/prima/firmware_bin

PRODUCT_COPY_FILES += $(PRIMA_FW_DIR)/WCNSS_qcom_wlan_nv.bin:persist/WCNSS_qcom_wlan_nv.bin
PRODUCT_COPY_FILES += $(PRIMA_FW_DIR)/WCNSS_cfg.dat:system/etc/firmware/wlan/prima/WCNSS_cfg.dat
PRODUCT_COPY_FILES += $(PRIMA_FW_DIR)/WCNSS_qcom_cfg.ini:system/etc/firmware/wlan/prima/WCNSS_qcom_cfg.ini

# Build prima_wlan.ko
###########################################################
LOCAL_PATH := $(call my-dir)

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_PRIMA=../$(WLAN_BLD_DIR)/prima
# We are actually building wlan.ko here, as per the
# requirement we are specifying prima_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to prima_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan

include $(CLEAR_VARS)
LOCAL_MODULE             := prima_wlan.ko
LOCAL_MODULE_KBUILD_NAME := wlan.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/prima
include $(WLAN_BLD_DIR)/AndroidKernelModule.mk
###########################################################

#Create symbolic link
WLAN_PRIMA_SYMLINK := $(TARGET_OUT)/lib/modules/wlan.ko
$(WLAN_PRIMA_SYMLINK):
	@mkdir -p $(dir $@)
	ln -sf /system/lib/modules/prima/prima_wlan.ko $@

file := $(WLAN_PRIMA_SYMLINK)
ALL_PREBUILT += $(file)

WLAN_NV_FILE_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin
$(WLAN_NV_FILE_SYMLINK):
	@mkdir -p $(dir $@)
	ln -s -f /persist/WCNSS_qcom_wlan_nv.bin $@

file := $(WLAN_NV_FILE_SYMLINK)
ALL_PREBUILT += $(file)
