
# Android makefile for the WLAN Libra Module

# Build/Package only in case of 7x30
ifneq (, $(filter msm7630_surf, $(TARGET_PRODUCT)))

PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/qcom_fw.bin:system/etc/firmware/wlan/volans/qcom_fw.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/qcom_wlan_nv.bin:persist/qcom_wlan_nv.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/cfg.dat:system/etc/firmware/wlan/volans/cfg.dat
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/qcom_cfg.ini:system/etc/firmware/wlan/volans/qcom_cfg.ini

ACP_BINARY_OUT := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER := $(TARGET_OUT)/lib/modules
WLAN_RF_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/volans_rf.ko
WLAN_RF_FTM_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/volans_rf_ftm.ko
WLAN_RF_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/volans_rf.ko
WLAN_RF_FTM_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/volans_rf_ftm.ko
WLAN_RF_LIBRA_SDIOIF_OUT :=  $(TARGET_OUT)/lib/modules/librasdioif.ko

file := $(WLAN_RF_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_FTM_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_FTM_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_LIBRA_SDIOIF_OUT)
ALL_PREBUILT += $(file)

$(MAKE_MODULES_FOLDER) :
	mkdir -p $(MAKE_MODULES_FOLDER)

$(WLAN_RF_OUT): kernel $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL)
	$(MAKE) -C kernel M=../vendor/qcom/proprietary/wlan/volans/CORE/HDD/src O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- 

$(WLAN_RF_FTM_OUT): $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_RF_OUT)
	$(MAKE) -C kernel M=../vendor/qcom/proprietary/wlan/volans/ftm/CORE/HDD/src O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- BUILD_FTM_DRIVER=1

$(WLAN_RF_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_OUT) $(MAKE_MODULES_FOLDER) 
	$(ACP) -f $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/volans_rf.ko $(WLAN_RF_PRODUCT_OUT)

$(WLAN_RF_FTM_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_PRODUCT_OUT) $(WLAN_RF_FTM_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/ftm/CORE/HDD/src/volans_rf_ftm.ko $(WLAN_RF_FTM_PRODUCT_OUT)
	
$(WLAN_RF_LIBRA_SDIOIF_OUT): $(ACP_BINARY_OUT) $(WLAN_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(KERNEL_OUT)/drivers/net/wireless/libra/librasdioif.ko $(WLAN_RF_LIBRA_SDIOIF_OUT)

all: $(WLAN_RF_OUT) $(WLAN_RF_FTM_OUT) $(WLAN_RF_PRODUCT_OUT) $(WLAN_RF_FTM_PRODUCT_OUT) $(WLAN_RF_LIBRA_SDIOIF_OUT) $(MAKE_SYMBOLIC_LINK)

endif
