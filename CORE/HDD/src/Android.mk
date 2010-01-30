
# Android makefile for the WLAN Libra Module

ifneq (, $(filter msm7627_surf msm7627_ffa msm7630_surf, $(TARGET_PRODUCT)))
PRODUCT_COPY_FILES += vendor/qcom-proprietary/wlan/firmware_bin/qcom_fw.bin:system/etc/firmware/wlan/qcom_fw.bin
PRODUCT_COPY_FILES += vendor/qcom-proprietary/wlan/firmware_bin/qcom_nv.bin:system/etc/firmware/wlan/qcom_nv.bin
PRODUCT_COPY_FILES += vendor/qcom-proprietary/wlan/firmware_bin/cfg.dat:system/etc/firmware/wlan/cfg.dat
PRODUCT_COPY_FILES += vendor/qcom-proprietary/wlan/firmware_bin/qcom_cfg.ini:system/etc/firmware/wlan/qcom_cfg.ini

MAKE_MODULES_FOLDER := out/target/product/$(TARGET_PRODUCT)/system/lib/modules
WLAN_OUT := vendor/qcom-proprietary/wlan/CORE/HDD/src/libra.o
WLAN_FTM_OUT := vendor/qcom-proprietary/wlan/ftm/CORE/HDD/src/libra_ftm.o
WLAN_PRODUCT_OUT := out/target/product/$(TARGET_PRODUCT)/system/lib/modules/libra.ko
WLAN_FTM_PRODUCT_OUT := out/target/product/$(TARGET_PRODUCT)/system/lib/modules/libra_ftm.ko
WLAN_LIBRA_SDIOIF_OUT := out/target/product/$(TARGET_PRODUCT)/system/lib/modules/librasdioif.ko

file := $(WLAN_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_FTM_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_FTM_PRODUCT_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_LIBRA_SDIOIF_OUT)
ALL_PREBUILT += $(file)

$(MAKE_MODULES_FOLDER) :
	mkdir -p $(MAKE_MODULES_FOLDER)

$(WLAN_OUT):  $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL)
	$(MAKE) -C kernel M=../vendor/qcom-proprietary/wlan/CORE/HDD/src O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- 

$(WLAN_FTM_OUT):  $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_OUT)
	$(MAKE) -C kernel M=../vendor/qcom-proprietary/wlan/ftm/CORE/HDD/src O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- BUILD_FTM_DRIVER=1
	
# Package only incase of 7x30 target
$(WLAN_PRODUCT_OUT): $(WLAN_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f out/target/product/$(TARGET_PRODUCT)/obj/vendor/qcom-proprietary/wlan/CORE/HDD/src/libra.ko $(WLAN_PRODUCT_OUT)

$(WLAN_FTM_PRODUCT_OUT): $(WLAN_PRODUCT_OUT) $(WLAN_FTM_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f out/target/product/$(TARGET_PRODUCT)/obj/vendor/qcom-proprietary/wlan/ftm/CORE/HDD/src/libra_ftm.ko $(WLAN_FTM_PRODUCT_OUT)
	
$(WLAN_LIBRA_SDIOIF_OUT): $(WLAN_OUT) $(MAKE_MODULES_FOLDER)
	$(ACP) -f $(KERNEL_OUT)/drivers/net/wireless/libra/librasdioif.ko $(WLAN_LIBRA_SDIOIF_OUT)

all: $(WLAN_OUT) $(WLAN_FTM_OUT) $(WLAN_PRODUCT_OUT) $(WLAN_FTM_PRODUCT_OUT) $(WLAN_LIBRA_SDIOIF_OUT) $(MAKE_SYMBOLIC_LINK)

endif
