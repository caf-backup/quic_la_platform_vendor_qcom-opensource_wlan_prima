
# Android makefile for the WLAN Libra Module

# Build/Package only in case of 7x30 and 7x27 target
ifneq (, $(filter msm7627_surf msm7627_ffa msm7627_7x_ffa msm7627_7x_surf msm7630_surf msm7630_fusion, $(QCOM_TARGET_PRODUCT)))

WLAN     := $(shell echo $(my-dir) | sed -e "s/.*vendor/vendor/" \
				         -e "s/\/CORE.*//")
WLAN_OUT := $(TARGET_OUT_INTERMEDIATES)/$(WLAN)

PRODUCT_COPY_FILES += $(WLAN)/firmware_bin/qcom_fw.bin:system/etc/firmware/wlan/qcom_fw.bin
PRODUCT_COPY_FILES += $(WLAN)/firmware_bin/qcom_wapi_fw.bin:system/etc/firmware/wlan/qcom_wapi_fw.bin
PRODUCT_COPY_FILES += $(WLAN)/firmware_bin/qcom_wlan_nv.bin:persist/qcom_wlan_nv.bin
PRODUCT_COPY_FILES += $(WLAN)/firmware_bin/cfg.dat:system/etc/firmware/wlan/cfg.dat
PRODUCT_COPY_FILES += $(WLAN)/firmware_bin/qcom_cfg.ini:system/etc/firmware/wlan/qcom_cfg.ini

ACP_BINARY_OUT 	      := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER   := $(TARGET_OUT)/lib/modules
WLAN_PRODUCT_OUT      := $(TARGET_OUT)/lib/modules/libra.ko
WLAN_FTM_PRODUCT_OUT  := $(TARGET_OUT)/lib/modules/libra_ftm.ko
WLAN_LIBRA_SDIOIF_OUT := $(TARGET_OUT)/lib/modules/librasdioif.ko
WLAN_RF_LIBRA_OUT     := $(WLAN_OUT)/CORE/HDD/src/libra.ko
WLAN_RF_LIBRA_MDIR    := ../$(WLAN)/CORE/HDD/src
WLAN_RF_FTM_LIBRA_OUT := $(WLAN_OUT)/ftm/CORE/HDD/src/libra_ftm.ko
WLAN_RF_FTM_LIBRA_MDIR:= ../$(WLAN)/ftm/CORE/HDD/src

file := $(WLAN_RF_LIBRA_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_FTM_LIBRA_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_FTM_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_LIBRA_SDIOIF_OUT)
ALL_PREBUILT += $(file)

$(MAKE_MODULES_FOLDER) :
	mkdir -p $(MAKE_MODULES_FOLDER)

$(WLAN_RF_LIBRA_MDIR): $(KERNEL_OUT)
	mkdir -p $(KERNEL_OUT)/$(WLAN_RF_LIBRA_MDIR)

$(WLAN_RF_FTM_LIBRA_MDIR): $(KERNEL_OUT)
	mkdir -p $(KERNEL_OUT)/$(WLAN_RF_FTM_LIBRA_MDIR)

$(WLAN_RF_LIBRA_OUT): $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_RF_LIBRA_MDIR)
	$(MAKE) -C kernel M=$(WLAN_RF_LIBRA_MDIR) O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- WLAN=../$(WLAN)

$(WLAN_RF_FTM_LIBRA_OUT): $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_RF_LIBRA_OUT) $(WLAN_RF_FTM_LIBRA_MDIR)
	$(MAKE) -C kernel M=$(WLAN_RF_FTM_LIBRA_MDIR) O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- BUILD_FTM_DRIVER=1 WLAN=../$(WLAN)
	
$(WLAN_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_LIBRA_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(WLAN_RF_LIBRA_OUT) $(WLAN_PRODUCT_OUT)

$(WLAN_FTM_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_PRODUCT_OUT) $(WLAN_RF_FTM_LIBRA_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(WLAN_RF_FTM_LIBRA_OUT) $(WLAN_FTM_PRODUCT_OUT)
	
$(WLAN_LIBRA_SDIOIF_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_LIBRA_OUT) $(MAKE_MODULES_FOLDER) $(TARGET_PREBUILT_KERNEL)
	$(ACP) -f $(KERNEL_OUT)/drivers/net/wireless/libra/librasdioif.ko $(WLAN_LIBRA_SDIOIF_OUT)

all: $(WLAN_RF_LIBRA_OUT) $(WLAN_RF_FTM_LIBRA_OUT) $(WLAN_PRODUCT_OUT) $(WLAN_FTM_PRODUCT_OUT) $(WLAN_LIBRA_SDIOIF_OUT) $(MAKE_SYMBOLIC_LINK)

endif
