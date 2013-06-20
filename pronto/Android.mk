# Android makefile for the WLAN WCNSS/Pronto Module

LOCAL_PATH := $(call my-dir)
WLAN_BLD_DIR := vendor/qcom/proprietary/wlan-noship
ifeq ($(call is-android-codename,JELLY_BEAN),true)
       DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
       DLKM_DIR := build/dlkm
endif

# Build proprietary_pronto_wlan.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_PRONTO=../$(WLAN_BLD_DIR)/pronto
KBUILD_OPTIONS += PRIMA_SRC_DIR=../$(WLAN_BLD_DIR)/prima
KBUILD_OPTIONS += WCNSS_DIR=../vendor/qcom/proprietary/wlan-noship/wcnss
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

include $(CLEAR_VARS)
LOCAL_MODULE              := proprietary_pronto_wlan.ko
LOCAL_MODULE_KBUILD_NAME  := wlan.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(TARGET_OUT)/lib/modules/pronto
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

