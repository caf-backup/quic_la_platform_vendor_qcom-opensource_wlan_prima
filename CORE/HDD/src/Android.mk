
# Android makefile for the WLAN Libra Module

# Build/Package only in case of 7x30 and 7x27 target
ifneq (, $(filter msm7627_surf msm7627_ffa msm7630_surf msm7630_fusion, $(QCOM_TARGET_PRODUCT)))

PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_fw.bin:system/etc/firmware/wlan/qcom_fw.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_wapi_fw.bin:system/etc/firmware/wlan/qcom_wapi_fw.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_wlan_nv.bin:persist/qcom_wlan_nv.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/cfg.dat:system/etc/firmware/wlan/cfg.dat
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/firmware_bin/qcom_cfg.ini:system/etc/firmware/wlan/qcom_cfg.ini

ACP_BINARY_OUT := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER := $(TARGET_OUT)/lib/modules/libra
WLAN_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/CORE/HDD/src/libra.ko
WLAN_TEMP_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/CORE/HDD/src/wlan.ko
WLAN_MDIR := ../vendor/qcom/proprietary/wlan/CORE/HDD/src
WLAN_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/libra/libra.ko
WLAN_LIBRA_SDIOIF_OUT :=  $(TARGET_OUT)/lib/modules/librasdioif.ko
ifneq (, $(filter msm7627_surf msm7627_ffa, $(QCOM_TARGET_PRODUCT)))
WLAN_WCN1312_SYMLINK := $(TARGET_OUT)/lib/modules/wlan.ko
endif


file := $(WLAN_OUT)
ALL_PREBUILT += $(file)
ifneq (, $(filter msm7627_surf msm7627_ffa, $(QCOM_TARGET_PRODUCT)))
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
ifneq (, $(filter msm7627_surf msm7627_ffa, $(QCOM_TARGET_PRODUCT)))
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
ifneq (, $(filter msm7627_surf msm7627_ffa, $(QCOM_TARGET_PRODUCT)))
	$(WLAN_WCN1312_SYMLINK)
endif


endif
