
# Android makefile for the WLAN Libra Module

# Build/Package only in case of 7x30 and 7x27 target
ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627_ffa msm7630_surf msm7630_fusion msm7627_6x),true)

LOCAL_PATH := $(call my-dir)

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
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware/wlan
LOCAL_SRC_FILES    := ../../../firmware_bin/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ACP_BINARY_OUT := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER := $(TARGET_OUT)/lib/modules/libra
WLAN_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/CORE/HDD/src/libra.ko
WLAN_TEMP_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/CORE/HDD/src/wlan.ko
WLAN_MDIR := ../vendor/qcom/proprietary/wlan/CORE/HDD/src
WLAN_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/libra/libra.ko
WLAN_LIBRA_SDIOIF_OUT :=  $(TARGET_OUT)/lib/modules/librasdioif.ko
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
WLAN_WCN1312_SYMLINK := $(TARGET_OUT)/lib/modules/wlan.ko
endif


file := $(WLAN_OUT)
ALL_PREBUILT += $(file)
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
file := $(WLAN_WCN1312_SYMLINK)
ALL_PREBUILT += $(file)
endif
file := $(WLAN_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_LIBRA_SDIOIF_OUT)
ALL_PREBUILT += $(file)

$(MAKE_MODULES_FOLDER) :
	mkdir -p $(MAKE_MODULES_FOLDER)

$(WLAN_MDIR): $(KERNEL_OUT)
	mkdir -p $(KERNEL_OUT)/$(WLAN_MDIR)

#POR for 7x27 is only libra
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
$(WLAN_WCN1312_SYMLINK): $(WLAN_RF_LIBRA_OUT) $(WLAN_PRODUCT_OUT) $(MAKE_MODULES_FOLDER)
	ln -s -f /system/lib/modules/libra/libra.ko $(WLAN_WCN1312_SYMLINK)
endif

$(WLAN_TEMP_OUT): kernel $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_MDIR)
	$(MAKE) -C kernel M=$(WLAN_MDIR) O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- MODNAME=wlan

$(WLAN_OUT): $(WLAN_TEMP_OUT)
	mv -f $(WLAN_TEMP_OUT) $(WLAN_OUT)

$(WLAN_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(WLAN_OUT) $(WLAN_PRODUCT_OUT)

$(WLAN_LIBRA_SDIOIF_OUT): $(ACP_BINARY_OUT) $(WLAN_OUT) $(MAKE_MODULES_FOLDER) $(TARGET_PREBUILT_KERNEL)
	$(ACP) -f $(KERNEL_OUT)/drivers/net/wireless/libra/librasdioif.ko $(WLAN_LIBRA_SDIOIF_OUT)

all: $(WLAN_OUT) $(WLAN_PRODUCT_OUT) $(WLAN_LIBRA_SDIOIF_OUT) $(MAKE_SYMBOLIC_LINK)
ifeq ($(call is-chipset-in-board-platform,msm7627),true)
	$(WLAN_WCN1312_SYMLINK)
endif


endif
