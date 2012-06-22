ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)
define include-ar6004-prebuilt
    include $$(CLEAR_VARS)
    LOCAL_MODULE := $(4)
    LOCAL_MODULE_STEM := $(3)
    LOCAL_MODULE_TAGS := debug optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(2)
    LOCAL_SRC_FILES := $(1)
    include $$(BUILD_PREBUILT)
endef

define add-ar6004-prebuilt-file
    $(eval $(include-ar6004-prebuilt))
endef

# HW1.2 firmware
ar6004_hw12_dst_dir := $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.2
$(call add-ar6004-prebuilt-file,hw1.2/fw.ram.bin,$(ar6004_hw12_dst_dir),fw.ram.bin,ar6004_fw_12)
$(call add-ar6004-prebuilt-file,hw1.2/bdata.bin,$(ar6004_hw12_dst_dir),bdata.bin,ar6004_bdata_12)
ar6004_hw12_dst_dir :=

# HW1.3 firmware
ar6004_hw13_dst_dir := $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.3
$(call add-ar6004-prebuilt-file,hw1.3/fw.ram.bin,$(ar6004_hw13_dst_dir),fw.ram.bin,ar6004_fw_13)
$(call add-ar6004-prebuilt-file,hw1.3/bdata.bin,$(ar6004_hw13_dst_dir),bdata.bin,ar6004_bdata_13)
ar6004_hw13_dst_dir :=

endif
