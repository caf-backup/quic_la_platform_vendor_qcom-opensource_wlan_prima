# Android makefile for the WLAN WCNSS/Prima Module

# Build/Package only in case of 8960 target

LOCAL_PATH := $(call my-dir)
WLAN_BLD_DIR := vendor/qcom/proprietary/wlan
ifeq ($(call is-android-codename,JELLY_BEAN),true)
       DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
       DLKM_DIR := build/dlkm
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := WCNSS1_2_qcom_wlan_nv.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := WCNSS1_2_cfg.dat
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan/prima1_2
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := WCNSS1_2_qcom_cfg.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(PRODUCT_OUT)/persist
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# Build prima1_2_wlan.ko
###########################################################

# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_PRIMA=../$(WLAN_BLD_DIR)/prima1_2

# We are actually building wlan.ko here, as per the
# requirement we are specifying prima_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to prima_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

include $(CLEAR_VARS)
LOCAL_MODULE              := prima1_2_wlan.ko
LOCAL_MODULE_KBUILD_NAME  := wlan.ko
LOCAL_MODULE_TAGS         := debug
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(TARGET_OUT)/lib/modules/prima1_2
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

MV_CFG80211_MODULE := $(KERNEL_MODULES_OUT)/prima/cfg80211.ko
$(MV_CFG80211_MODULE): CFG80211_MODULE := $(KERNEL_MODULES_OUT)/cfg80211.ko
$(MV_CFG80211_MODULE): $(TARGET_PREBUILT_INT_KERNEL)
	@mkdir -p $(dir $@)
	@mv -f $(CFG80211_MODULE) $@

ALL_DEFAULT_INSTALLED_MODULES += $(MV_CFG80211_MODULE)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(MV_CFG80211_MODULE)

$(shell mkdir -p $(TARGET_OUT_ETC)/firmware/wlan/prima1_2; \
        ln -sf /persist/WCNSS1_2_qcom_wlan_nv.bin \
        $(TARGET_OUT_ETC)/firmware/wlan/prima1_2/WCNSS1_2_qcom_wlan_nv.bin; \
        ln -sf /data/misc/wifi/WCNSS1_2_qcom_cfg.ini \
        $(TARGET_OUT_ETC)/firmware/wlan/prima1_2/WCNSS1_2_qcom_cfg.ini)
