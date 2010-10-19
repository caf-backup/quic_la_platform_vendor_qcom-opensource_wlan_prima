
# Android makefile for the WLAN Libra Module

# Build/Package only in case of 7x30 and 7x27 target
ifneq (, $(filter msm7627_surf msm7627_ffa msm7627_7x_ffa msm7627_7x_surf msm7630_surf msm7630_fusion, $(QCOM_TARGET_PRODUCT)))

PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_fw.bin:system/etc/firmware/wlan/qcom_fw.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_wlan_nv.bin:persist/qcom_wlan_nv.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/cfg.dat:system/etc/firmware/wlan/cfg.dat
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_cfg.ini:system/etc/firmware/wlan/qcom_cfg.ini

ACP_BINARY_OUT := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER := $(TARGET_OUT)/lib/modules
WLAN_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/CORE/HDD/src/libra.ko
WLAN_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/libra.ko
WLAN_LIBRA_SDIOIF_OUT :=  $(TARGET_OUT)/lib/modules/librasdioif.ko

file := $(WLAN_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_LIBRA_SDIOIF_OUT)
ALL_PREBUILT += $(file)

$(MAKE_MODULES_FOLDER) :
	mkdir -p $(MAKE_MODULES_FOLDER)

$(WLAN_OUT): kernel $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL)
	$(MAKE) -C kernel M=../vendor/qcom/proprietary/wlan/CORE/HDD/src O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- 

$(WLAN_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/CORE/HDD/src/libra.ko $(WLAN_PRODUCT_OUT)

$(WLAN_LIBRA_SDIOIF_OUT): $(ACP_BINARY_OUT) $(WLAN_OUT) $(MAKE_MODULES_FOLDER) $(TARGET_PREBUILT_KERNEL)
	$(ACP) -f $(KERNEL_OUT)/drivers/net/wireless/libra/librasdioif.ko $(WLAN_LIBRA_SDIOIF_OUT)

all: $(WLAN_OUT) $(WLAN_PRODUCT_OUT) $(WLAN_LIBRA_SDIOIF_OUT) $(MAKE_SYMBOLIC_LINK)

endif
