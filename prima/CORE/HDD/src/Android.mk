
# Android makefile for the WLAN WCNSS/Prima Module

# NB: Firmware names and locations have not yet been updated for Prima
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/prima/firmware_bin/WCN1314_qcom_wlan_nv.bin:persist/WCN1314_qcom_wlan_nv.bin
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/prima/firmware_bin/WCN1314_cfg.dat:system/etc/firmware/wlan/volans/WCN1314_cfg.dat
PRODUCT_COPY_FILES += vendor/qcom/proprietary/wlan/prima/firmware_bin/WCN1314_qcom_cfg.ini:system/etc/firmware/wlan/volans/WCN1314_qcom_cfg.ini

ACP_BINARY_OUT := $(HOST_OUT)/bin/acp
MAKE_MODULES_FOLDER_PRIMA := $(TARGET_OUT)/lib/modules/prima

# Build it as wlan.ko and then move it to prima_wlan.ko

WLAN_RF_PRIMA_TEMP_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/prima/CORE/HDD/src/wlan.ko
WLAN_RF_PRIMA_OUT := $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/proprietary/wlan/prima/CORE/HDD/src/prima_wlan.ko
WLAN_RF_PRIMA_MDIR := ../vendor/qcom/proprietary/wlan/prima/CORE/HDD/src


WLAN_RF_PRODUCT_OUT := $(TARGET_OUT)/lib/modules/prima/prima_wlan.ko

# NB: Firmware names and locations have not yet been updated for Prima
WLAN_NV_FILE_SYMLINK := $(TARGET_OUT)/etc/firmware/wlan/volans/WCN1314_qcom_wlan_nv.bin

file := $(WLAN_NV_FILE_SYMLINK)
ALL_PREBUILT += $(file)

WLAN_KO_FILE_SYMLINK := $(TARGET_OUT)/lib/modules/wlan.ko

file := $(WLAN_KO_FILE_SYMLINK)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_PRIMA_OUT)
ALL_PREBUILT += $(file)

file := $(WLAN_RF_PRODUCT_OUT)
ALL_PREBUILT += $(file)

PERSIST_FOLDER := ./out/target/product/$(QCOM_TARGET_PRODUCT)/persist
$(PERSIST_FOLDER):
	mkdir -p $(PERSIST_FOLDER)

# NB: Firmware names and locations have not yet been updated for Prima
FW_FOLDER := $(TARGET_OUT)/etc/firmware/wlan/volans
$(FW_FOLDER):
	mkdir -p $(TARGET_OUT)/etc/firmware/wlan/volans

# NB: Firmware names and locations have not yet been updated for Prima
$(WLAN_NV_FILE_SYMLINK): $(PERSIST_FOLDER) $(FW_FOLDER)
	cp -f  ./vendor/qcom/proprietary/wlan/prima/firmware_bin/WCN1314_qcom_wlan_nv.bin $(PERSIST_FOLDER)
	ln -s -f /persist/WCN1314_qcom_wlan_nv.bin $(WLAN_NV_FILE_SYMLINK)

$(WLAN_KO_FILE_SYMLINK): $(WLAN_RF_PRIMA_OUT) $(WLAN_RF_PRODUCT_OUT) $(MAKE_MODULES_FOLDER_PRIMA)
	ln -s -f /system/lib/modules/prima/prima_wlan.ko $(WLAN_KO_FILE_SYMLINK)

$(MAKE_MODULES_FOLDER_PRIMA) :
	mkdir -p $(MAKE_MODULES_FOLDER_PRIMA)

$(WLAN_RF_PRIMA_MDIR): $(KERNEL_OUT)
	mkdir -p $(KERNEL_OUT)/$(WLAN_RF_PRIMA_MDIR)

$(WLAN_RF_PRIMA_TEMP_OUT): kernel $(KERNEL_OUT) $(KERNEL_CONFIG) $(TARGET_PREBUILT_KERNEL) $(WLAN_RF_PRIMA_MDIR) 
	$(MAKE) -C kernel M=$(WLAN_RF_PRIMA_MDIR) O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- MODNAME=wlan

$(WLAN_RF_PRIMA_OUT): $(WLAN_RF_PRIMA_TEMP_OUT)
	mv -f $(WLAN_RF_PRIMA_TEMP_OUT) $(WLAN_RF_PRIMA_OUT)

$(WLAN_RF_PRODUCT_OUT): $(ACP_BINARY_OUT) $(WLAN_RF_PRIMA_OUT) $(MAKE_MODULES_FOLDER_PRIMA)
	$(ACP) -f $(WLAN_RF_PRIMA_OUT) $(WLAN_RF_PRODUCT_OUT)

all: $(WLAN_RF_PRIMA_OUT) $(WLAN_RF_PRODUCT_OUT) $(MAKE_SYMBOLIC_LINK) $(WLAN_NV_FILE_SYMLINK) 	$(WLAN_KO_FILE_SYMLINK)
