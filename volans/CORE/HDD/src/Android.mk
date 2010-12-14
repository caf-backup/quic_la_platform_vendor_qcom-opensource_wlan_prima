
# Android makefile for the WLAN WCN1314 Module

# Build/Package only in case of 7x30 and msm8660
ifneq (, $(filter msm7630_surf msm8660_surf msm8660_csfb, $(QCOM_TARGET_PRODUCT)))

PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/WCN1314_qcom_fw.bin:system/etc/firmware/wlan/volans/WCN1314_qcom_fw.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/WCN1314_qcom_wlan_nv.bin:persist/WCN1314_qcom_wlan_nv.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/WCN1314_cfg.dat:system/etc/firmware/wlan/volans/WCN1314_cfg.dat
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/volans/firmware_bin/WCN1314_qcom_cfg.ini:system/etc/firmware/wlan/volans/WCN1314_qcom_cfg.ini

ACP_BINARY_OUT := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER := $(TARGET_OUT)/lib/modules

# Build it as libra.ko and then move it to WCN1314_rf.ko, thus rmmod will work.
WLAN_RF_VOLANS_TEMP_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/libra.ko
WLAN_RF_VOLANS_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/WCN1314_rf.ko
WLAN_RF_VOLANS_MDIR := ../vendor/qcom/proprietary/wlan/volans/CORE/HDD/src

WLAN_RF_FTM_VOLANS_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/volans/ftm/CORE/HDD/src/WCN1314_rf_ftm.ko
WLAN_RF_FTM_VOLANS_MDIR := ../vendor/qcom/proprietary/wlan/volans/ftm/CORE/HDD/src

WLAN_RF_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/WCN1314_rf.ko
WLAN_RF_FTM_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/WCN1314_rf_ftm.ko
WLAN_RF_LIBRA_SDIOIF_OUT :=  $(TARGET_OUT)/lib/modules/librasdioif.ko

file := $(WLAN_RF_VOLANS_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_FTM_VOLANS_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_FTM_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_LIBRA_SDIOIF_OUT)
ALL_PREBUILT += $(file)

$(MAKE_MODULES_FOLDER) :
	mkdir -p $(MAKE_MODULES_FOLDER)

$(WLAN_RF_VOLANS_MDIR): $(KERNEL_OUT)
	mkdir -p $(KERNEL_OUT)/$(WLAN_RF_VOLANS_MDIR)

$(WLAN_RF_FTM_VOLANS_MDIR): $(KERNEL_OUT)
	mkdir -p $(KERNEL_OUT)/$(WLAN_RF_FTM_VOLANS_MDIR)

$(WLAN_RF_VOLANS_TEMP_OUT): kernel $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_RF_VOLANS_MDIR) 
	$(MAKE) -C kernel M=$(WLAN_RF_VOLANS_MDIR) O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- MODNAME=libra

$(WLAN_RF_VOLANS_OUT): $(WLAN_RF_VOLANS_TEMP_OUT)
	mv -f $(WLAN_RF_VOLANS_TEMP_OUT) $(WLAN_RF_VOLANS_OUT)

$(WLAN_RF_FTM_VOLANS_OUT): $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_RF_VOLANS_OUT) $(WLAN_RF_FTM_VOLANS_MDIR)
	$(MAKE) -C kernel M=$(WLAN_RF_FTM_VOLANS_MDIR) O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- BUILD_FTM_DRIVER=1

$(WLAN_RF_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_VOLANS_OUT) $(MAKE_MODULES_FOLDER) 
	$(ACP) -f $(WLAN_RF_VOLANS_OUT) $(WLAN_RF_PRODUCT_OUT)

$(WLAN_RF_FTM_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_PRODUCT_OUT) $(WLAN_RF_FTM_VOLANS_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(WLAN_RF_FTM_VOLANS_OUT) $(WLAN_RF_FTM_PRODUCT_OUT)
	
$(WLAN_RF_LIBRA_SDIOIF_OUT): $(ACP_BINARY_OUT) $(WLAN_OUT) $(MAKE_MODULES_FOLDER) $(TARGET_PREBUILT_KERNEL)
	$(ACP) -f $(KERNEL_OUT)/drivers/net/wireless/libra/librasdioif.ko $(WLAN_RF_LIBRA_SDIOIF_OUT)

all: $(WLAN_RF_VOLANS_OUT) $(WLAN_RF_FTM_VOLANS_OUT) $(WLAN_RF_PRODUCT_OUT) $(WLAN_RF_FTM_PRODUCT_OUT) $(WLAN_RF_LIBRA_SDIOIF_OUT) $(MAKE_SYMBOLIC_LINK)

endif
