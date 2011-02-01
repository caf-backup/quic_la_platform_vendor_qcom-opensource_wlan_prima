KERNEL_DIR ?= ../../../../../../../../kernel
KERNEL_OUT ?= ../out/target/product/$(TARGET_PRODUCT)/obj/KERNEL_OBJ

TOOLCHAIN_PREFIX ?= arm-eabi-

MAKE_FLAGS := -C $(KERNEL_DIR) O=$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=$(TOOLCHAIN_PREFIX)

#Whether to build debug version
BUILD_DEBUG_VERSION := 1

#Enable this flag to build driver in diag version
BUILD_DIAG_VERSION := 0
#Enable this flag to build ftm driver
BUILD_FTM_DRIVER := 0

#KERNEL include dir
KERNEL_INC ?= ./include/linux

ifndef WLAN
WLAN := ../vendor/qcom/proprietary/wlan/libra/
endif

WLAN_DIR 		:= $(WLAN)/CORE
SAL_EXTERNAL_DIR 	:= $(WLAN)/external
WLANFW_DIR 		:= $(WLAN)/wlanfw

############## HDD ###################
HDD_INC := $(WLAN_DIR)/HDD/inc/

HDD_OBJS := wlan_hdd_main.o  \
            wlan_hdd_scan.o  \
            wlan_hdd_assoc.o  \
            wlan_hdd_mib.o  \
            wlan_hdd_wext.o \
            wlan_hdd_tx_rx.o   \
            wlan_hdd_dp_utils.o \
            wlan_hdd_early_suspend.o  \
            wlan_hdd_wmm.o \
	    wlan_hdd_cfg.o \
            wlan_hdd_wowl.o \
            wlan_hdd_ftm.o  \
            wlan_hdd_hostapd.o  \
            wlan_hdd_softap_tx_rx.o

ifeq ($(CONFIG_CFG80211), y)
HDD_OBJS += wlan_hdd_cfg80211.o
endif

############# SAP ##################### 
SAP_INC := -I$(WLAN_DIR)/SAP/inc \
           -I$(WLAN_DIR)/SAP/src

SAP_SRC_DIR := ../../SAP/src

SAP_OBJS := $(SAP_SRC_DIR)/sapApiLinkCntl.o \
	$(SAP_SRC_DIR)/sapFsm.o \
	$(SAP_SRC_DIR)/sapModule.o\
	$(SAP_SRC_DIR)/sapChSelect.o

############## VOSS ################## 
VOSS_INC :=  	-I$(WLAN_DIR)/VOSS/inc/ \
		-I$(WLAN_DIR)/VOSS/inc/Linux \
		-I$(SAL_EXTERNAL_DIR) \
		-I$(WLAN_DIR)/VOSS/src

VOSS_SRC_DIR := ../../VOSS/src
VOSS_OBJS :=    $(VOSS_SRC_DIR)/vos_types.o \
                $(VOSS_SRC_DIR)/vos_event.o  \
                $(VOSS_SRC_DIR)/vos_getBin.o \
                $(VOSS_SRC_DIR)/vos_list.o   \
                $(VOSS_SRC_DIR)/vos_lock.o   \
                $(VOSS_SRC_DIR)/vos_memory.o \
                $(VOSS_SRC_DIR)/vos_mq.o     \
                $(VOSS_SRC_DIR)/vos_nvitem.o \
                $(VOSS_SRC_DIR)/vos_packet.o \
                $(VOSS_SRC_DIR)/vos_power.o  \
                $(VOSS_SRC_DIR)/vos_threads.o \
                $(VOSS_SRC_DIR)/vos_timer.o   \
                $(VOSS_SRC_DIR)/vos_trace.o  \
                $(VOSS_SRC_DIR)/vos_api.o    \
                $(VOSS_SRC_DIR)/vos_sched.o   \
                $(VOSS_SRC_DIR)/wlan_qct_bal.o \
                $(VOSS_SRC_DIR)/wlan_qct_sal.o

ifeq ($(BUILD_DIAG_VERSION),1)
VOSS_OBJS += $(VOSS_SRC_DIR)/vos_diag.o
endif

############# TL ##################### 
TL_INC := 	-I$(WLAN_DIR)/TL/inc \
		-I$(WLAN_DIR)/TL/src

TL_SRC_DIR := ../../TL/src
TL_OBJS := 	$(TL_SRC_DIR)/wlan_qct_tl.o \
		$(TL_SRC_DIR)/wlan_qct_tl_ba.o \
		$(TL_SRC_DIR)/wlan_qct_tl_hosupport.o

############# SYS ##################### 
SYS_INC := 	-I$(WLAN_DIR)/SYS/common/inc \
		-I$(WLAN_DIR)/SYS/legacy/src/pal/inc \
		-I$(WLAN_DIR)/SYS/legacy/src/platform/inc \
		-I$(WLAN_DIR)/SYS/legacy/src/system/inc \
		-I$(WLAN_DIR)/SYS/legacy/src/utils/inc \

SYS_COMMON_SRC_DIR := ../../SYS/common/src
SYS_LEGACY_SRC_DIR := ../../SYS/legacy/src
SYS_OBJS :=	$(SYS_COMMON_SRC_DIR)/wlan_qct_sys.o \
		$(SYS_LEGACY_SRC_DIR)/pal/src/palApiComm.o \
		$(SYS_LEGACY_SRC_DIR)/pal/src/palApiSdio.o \
		$(SYS_LEGACY_SRC_DIR)/pal/src/palTimer.o \
		$(SYS_LEGACY_SRC_DIR)/platform/src/VossWrapper.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/macInitApi.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/sysEntryFunc.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/sysWinStartup.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/dot11f.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/logApi.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/logDump.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/macTrace.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/parserApi.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/utilsApi.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/utilsParser.o 

############# MAC ##################### 
MAC_INC := 	-I$(WLAN_DIR)/MAC/inc \
		-I$(WLAN_DIR)/MAC/src/include \
		-I$(WLAN_DIR)/MAC/src/pe/include \
		-I$(WLAN_DIR)/MAC/src/dph \
		-I$(WLAN_DIR)/MAC/src/pe/lim \
		-I$(WLAN_DIR)/MAC/src/dvt

MAC_SRC_DIR :=  ../../MAC/src

MAC_CFG_OBJS := $(MAC_SRC_DIR)/cfg/cfgApi.o \
		$(MAC_SRC_DIR)/cfg/cfgProcMsg.o \
		$(MAC_SRC_DIR)/cfg/cfgSendMsg.o \
		$(MAC_SRC_DIR)/cfg/cfgDebug.o \
		$(MAC_SRC_DIR)/cfg/cfgParamName.o

MAC_DPH_OBJS := $(MAC_SRC_DIR)/dph/dphHashTable.o

MAC_LIM_OBJS := $(MAC_SRC_DIR)/pe/lim/limAIDmgmt.o               \
          	$(MAC_SRC_DIR)/pe/lim/limApi.o                   \
         	$(MAC_SRC_DIR)/pe/lim/limAssocUtils.o            \
          	$(MAC_SRC_DIR)/pe/lim/limIbssPeerMgmt.o          \
          	$(MAC_SRC_DIR)/pe/lim/limLinkMonitoringAlgo.o    \
          	$(MAC_SRC_DIR)/pe/lim/limProcessActionFrame.o    \
          	$(MAC_SRC_DIR)/pe/lim/limProcessAssocReqFrame.o  \
          	$(MAC_SRC_DIR)/pe/lim/limProcessAssocRspFrame.o  \
          	$(MAC_SRC_DIR)/pe/lim/limProcessAuthFrame.o      \
          	$(MAC_SRC_DIR)/pe/lim/limProcessBeaconFrame.o    \
          	$(MAC_SRC_DIR)/pe/lim/limProcessCfgUpdates.o     \
          	$(MAC_SRC_DIR)/pe/lim/limProcessDeauthFrame.o    \
          	$(MAC_SRC_DIR)/pe/lim/limProcessDisassocFrame.o  \
          	$(MAC_SRC_DIR)/pe/lim/limProcessLmmMessages.o    \
          	$(MAC_SRC_DIR)/pe/lim/limProcessMessageQueue.o   \
          	$(MAC_SRC_DIR)/pe/lim/limProcessMlmReqMessages.o \
          	$(MAC_SRC_DIR)/pe/lim/limProcessMlmRspMessages.o \
          	$(MAC_SRC_DIR)/pe/lim/limProcessProbeReqFrame.o  \
          	$(MAC_SRC_DIR)/pe/lim/limProcessProbeRspFrame.o  \
          	$(MAC_SRC_DIR)/pe/lim/limProcessSmeReqMessages.o \
          	$(MAC_SRC_DIR)/pe/lim/limPropExtsUtils.o         \
          	$(MAC_SRC_DIR)/pe/lim/limRoamingAlgo.o           \
          	$(MAC_SRC_DIR)/pe/lim/limScanResultUtils.o       \
          	$(MAC_SRC_DIR)/pe/lim/limSecurityUtils.o         \
          	$(MAC_SRC_DIR)/pe/lim/limSendManagementFrames.o  \
          	$(MAC_SRC_DIR)/pe/lim/limSendMessages.o          \
          	$(MAC_SRC_DIR)/pe/lim/limSendSmeRspMessages.o    \
          	$(MAC_SRC_DIR)/pe/lim/limSerDesUtils.o           \
          	$(MAC_SRC_DIR)/pe/lim/limSmeReqUtils.o           \
          	$(MAC_SRC_DIR)/pe/lim/limStaHashApi.o            \
          	$(MAC_SRC_DIR)/pe/lim/limTimerUtils.o            \
          	$(MAC_SRC_DIR)/pe/lim/limUtils.o                 \
          	$(MAC_SRC_DIR)/pe/lim/limLogDump.o               \
          	$(MAC_SRC_DIR)/pe/lim/limDebug.o	         \
                $(MAC_SRC_DIR)/pe/lim/limTrace.o		 \
          	$(MAC_SRC_DIR)/pe/lim/limAdmitControl.o \
		$(MAC_SRC_DIR)/pe/lim/limSession.o \
		$(MAC_SRC_DIR)/pe/lim/limSessionUtils.o

MAC_PMM_OBJS := $(MAC_SRC_DIR)/pe/pmm/pmmApi.o    \
                $(MAC_SRC_DIR)/pe/pmm/pmmAP.o    \
          	$(MAC_SRC_DIR)/pe/pmm/pmmDebug.o

MAC_SCH_OBJS := $(MAC_SRC_DIR)/pe/sch/schApi.o           \
          	$(MAC_SRC_DIR)/pe/sch/schBeaconGen.o     \
          	$(MAC_SRC_DIR)/pe/sch/schBeaconProcess.o \
          	$(MAC_SRC_DIR)/pe/sch/schDebug.o         \
          	$(MAC_SRC_DIR)/pe/sch/schMessage.o

MAC_OBJS :=     $(MAC_CFG_OBJS) \
                $(MAC_DPH_OBJS) \
                $(MAC_LIM_OBJS) \
                $(MAC_PMM_OBJS) \
                $(MAC_SCH_OBJS)

############# HAL ##################### 
HAL_INC := 	-I$(WLAN_DIR)/HAL/inc \
		-I$(WLAN_DIR)/HAL/src/halPhy \
		-I$(WLAN_DIR)/HAL/src/halFtm \
		-I$(WLAN_DIR)/HAL/src/halProc \
		-I$(WLAN_DIR)/HAL/src/halSys \
		-I$(WLAN_DIR)/HAL/src/halMac

HAL_SRC := ../../HAL/src

HAL_MAC_OBJS :=	$(HAL_SRC)/halMac/halAdu.o \
		$(HAL_SRC)/halMac/halApi.o \
		$(HAL_SRC)/halMac/halBmu.o \
		$(HAL_SRC)/halMac/halDPU.o \
		$(HAL_SRC)/halMac/halLogDump.o \
		$(HAL_SRC)/halMac/halMacBA.o \
		$(HAL_SRC)/halMac/halMacSecurityApi.o \
		$(HAL_SRC)/halMac/halMacStats.o \
		$(HAL_SRC)/halMac/halMacWmmApi.o \
		$(HAL_SRC)/halMac/halMailbox.o  \
		$(HAL_SRC)/halMac/halMCPU.o \
		$(HAL_SRC)/halMac/halMCU.o \
		$(HAL_SRC)/halMac/halMIF.o \
		$(HAL_SRC)/halMac/halMTU.o \
		$(HAL_SRC)/halMac/halRateTable.o \
		$(HAL_SRC)/halMac/halRpe.o \
		$(HAL_SRC)/halMac/halRxp.o \
		$(HAL_SRC)/halMac/halTpe.o \
		$(HAL_SRC)/halMac/halTxp.o    

HAL_PHY_OBJS :=	$(HAL_SRC)/halPhy/halPhy.o \
		$(HAL_SRC)/halPhy/phyTxPower.o \
		$(HAL_SRC)/halPhy/halPhyVos.o \
		$(HAL_SRC)/halPhy/asicAGC.o \
		$(HAL_SRC)/halPhy/asicWfm.o \
		$(HAL_SRC)/halPhy/asicTPC.o \
		$(HAL_SRC)/halPhy/rfUtils.o \
		$(HAL_SRC)/halPhy/phyDebug.o

		
HAL_FTM_OBJS :=	$(HAL_SRC)/halFtm/pttMsgApi.o \
		$(HAL_SRC)/halFtm/pttModule.o \
		$(HAL_SRC)/halFtm/rfGemini.o \
		$(HAL_SRC)/halFtm/asicTxFir.o \
		$(HAL_SRC)/halFtm/asicCal.o \
		$(HAL_SRC)/halFtm/asicTxCtl.o \
		$(HAL_SRC)/halFtm/pttFrameGen.o \
		$(HAL_SRC)/halFtm/asicPhyDbg.o \
		$(HAL_SRC)/halFtm/halPhyRates.o \
		$(HAL_SRC)/halFtm/halFtmRx.o


HAL_PROC_OBJS :=$(HAL_SRC)/halProc/halInitApi.o \
		$(HAL_SRC)/halProc/halAdaptThrsh.o \
		$(HAL_SRC)/halProc/halRateAdaptation.o \
		$(HAL_SRC)/halProc/halMsg.o \
		$(HAL_SRC)/halProc/hal.o \
		$(HAL_SRC)/halProc/halNimApi.o \
		$(HAL_SRC)/halProc/halMnt.o \
		$(HAL_SRC)/halProc/halStaTable.o \
		$(HAL_SRC)/halProc/halTimer.o \
		$(HAL_SRC)/halProc/halTx.o \
		$(HAL_SRC)/halProc/halUtils.o \
		$(HAL_SRC)/halProc/halTLApi.o \
		$(HAL_SRC)/halProc/halPwrSave.o \
		$(HAL_SRC)/halProc/halFwApi.o \
		$(HAL_SRC)/halProc/halRegBckup.o \
		$(HAL_SRC)/halProc/halRFBringup.o \
		$(HAL_SRC)/halProc/halDebug.o \
		$(HAL_SRC)/halProc/halBtc.o \
		$(HAL_SRC)/halProc/halTLFlush.o
 

HAL_SYS_OBJS := $(HAL_SRC)/halSys/halInterrupts.o \
		$(HAL_SRC)/halSys/halPMU.o \
		$(HAL_SRC)/halSys/halDXE.o \
		$(HAL_SRC)/halSys/halNv.o \
		$(HAL_SRC)/halSys/halNvTables.o \
		$(HAL_SRC)/halSys/halQFuse.o \
		$(HAL_SRC)/halSys/wlanHalAHB.o

HAL_OBJS :=	$(HAL_MAC_OBJS)  \
                $(HAL_PHY_OBJS)  \
                $(HAL_FTM_OBJS)  \
                $(HAL_PROC_OBJS) \
                $(HAL_SYS_OBJS)

############# SME ##################### 
SME_INC := 	-I$(WLAN_DIR)/SME/inc \
		-I$(WLAN_DIR)/SME/src/csr

SME_SRC_DIR := ../../SME/src

SME_CCM_OBJS := $(SME_SRC_DIR)/ccm/ccmApi.o \
		$(SME_SRC_DIR)/ccm/ccmLogDump.o

SME_CSR_OBJS := $(SME_SRC_DIR)/csr/csrLinkList.o \
		$(SME_SRC_DIR)/csr/csrApiScan.o \
		$(SME_SRC_DIR)/csr/csrApiRoam.o \
		$(SME_SRC_DIR)/csr/csrCmdProcess.o \
		$(SME_SRC_DIR)/csr/csrUtil.o \
		$(SME_SRC_DIR)/csr/csrLogDump.o

SME_PMC_OBJS := $(SME_SRC_DIR)/pmc/pmcApi.o \
		$(SME_SRC_DIR)/pmc/pmc.o \
		$(SME_SRC_DIR)/pmc/pmcLogDump.o

SME_QOS_OBJS := $(SME_SRC_DIR)/QoS/sme_Qos.o

SME_CMN_OBJS := $(SME_SRC_DIR)/sme_common/sme_Api.o

SME_BTC_OBJS := $(SME_SRC_DIR)/btc/btcApi.o

SME_OBJS := 	$(SME_CCM_OBJS) \
		$(SME_CSR_OBJS) \
		$(SME_PMC_OBJS) \
		$(SME_QOS_OBJS) \
		$(SME_CMN_OBJS) \
		$(SME_BTC_OBJS)

############# SSC ##################### 
SSC_INC := -I$(WLAN_DIR)/SSC/inc \
           -I$(WLAN_DIR)/SSC/src

SSC_SRC_DIR := ../../SSC/src

SSC_OBJS := $(SSC_SRC_DIR)/wlan_qct_ssc.o

############### SVC  #################

SVC_INC :=  	-I$(WLAN_DIR)/SVC/inc/
SVC_EXT_INC :=  -I$(WLAN_DIR)/SVC/external
NLINK_SRC_DIR := ../../SVC/src/nlink
BTC_SRC_DIR := ../../SVC/src/btc
PTT_SRC_DIR := ../../SVC/src/ptt
NLINK_OBJS :=    $(NLINK_SRC_DIR)/wlan_nlink_srv.o
BTC_OBJS :=      $(BTC_SRC_DIR)/wlan_btc_svc.o
PTT_OBJS :=      $(PTT_SRC_DIR)/wlan_ptt_sock_svc.o

EXTRA_CFLAGS += $(addprefix -I, $(KERNEL_INC))
EXTRA_CFLAGS += $(addprefix -I, $(HDD_INC))
#EXTRA_CFLAGS += $(BAP_INC)
EXTRA_CFLAGS += $(SAP_INC)
EXTRA_CFLAGS += $(VOSS_INC)
EXTRA_CFLAGS += $(TL_INC)
EXTRA_CFLAGS += $(SYS_INC)
EXTRA_CFLAGS += $(MAC_INC)
EXTRA_CFLAGS += $(HAL_INC)
EXTRA_CFLAGS += $(SME_INC)
EXTRA_CFLAGS += $(SSC_INC)
EXTRA_CFLAGS += $(SVC_INC)
EXTRA_CFLAGS += $(SVC_EXT_INC)
EXTRA_CFLAGS += -I$(WLANFW_DIR)/inc

EXTRA_LDFLAGS += --strip-debug

ifneq (,$(findstring arm-eabi-4.4, $(ANDROID_EABI_TOOLCHAIN)))
EXTRA_CFLAGS += -Wframe-larger-than=4096
endif


CDEFINES :=	-DANI_PRODUCT_TYPE_CLIENT=1 \
		-DANI_BUS_TYPE_SDIO=1 \
		-DANI_LITTLE_BYTE_ENDIAN \
		-DANI_LITTLE_BIT_ENDIAN \
		-DWLAN_STA=1 \
		-DAP=2 \
		-DWNI_POLARIS_FW_PRODUCT=1 \
		-DANI_CHIPSET_LIBRA \
		-DINTEGRATION_READY \
		-DVOSS_ENABLED \
		-DDOT11F_LITTLE_ENDIAN_HOST \
		-DGEN6_ONWARDS \
		-DANI_COMPILER_TYPE_GCC \
		-DANI_OS_TYPE_ANDROID=6 \
	 	-DWNI_POLARIS_FW_OS=6 \
    		-DADVANCED=3 \
    		-DWNI_POLARIS_FW_PACKAGE=9 \
		-DANI_LOGDUMP \
    		-DLIBRA_RF \
		-DPTT_SOCK_SVC_ENABLE \
		-DFEATURE_WLAN_UAPSD_FW_TRG_FRAMES \
		-DWLAN_SOFTAP_FEATURE \
		-DWLAN_FTM_STUB \
                -DFEATURE_WLAN_WAPI \
                -DLIBRA_WAPI_SUPPORT \
                -DWLAN_SDIO_DUMMY_CMD53_WORKAROUND \
		-D__linux__


ifeq ($(BUILD_DEBUG_VERSION),1)
CDEFINES += -DWLAN_DEBUG \
		-DTRACE_RECORD \
		-DPE_DEBUG_LOGW \
		-DPE_DEBUG_LOGE \
		-DDEBUG 
endif

ifeq ($(BUILD_DIAG_VERSION),1)
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT
endif

ifneq (, $(filter msm7630_surf msm7630_fusion, $(TARGET_PRODUCT)))
EXTRA_CFLAGS += -march=armv7-a
CDEFINES += -DMSM_PLATFORM_7x30
endif


ifneq (, $(filter msm7627_surf msm7627_7x_surf, $(TARGET_PRODUCT)))
EXTRA_CFLAGS += -march=armv6
endif


ifneq (, $(filter msm7627_ffa msm7627_7x_ffa, $(TARGET_PRODUCT)))
EXTRA_CFLAGS += -march=armv6
CDEFINES += -DMSM_PLATFORM_7x27_FFA
endif

ifeq ($(BUILD_FTM_DRIVER),1)
CDEFINES += -DANI_MANF_DIAG \
  	-UWLAN_FTM_STUB \
	-DANI_PHY_DEBUG
endif

KBUILD_CPPFLAGS += $(CDEFINES)


#                $(BAP_OBJS)
ifeq ($(BUILD_FTM_DRIVER),1)
obj-m += libra_ftm.o

libra_ftm-objs := 	$(VOSS_OBJS) \
                $(TL_OBJS)   \
                $(SSC_OBJS)  \
                $(SME_OBJS)  \
                $(SAP_OBJS)  \
		$(SYS_OBJS)  \
                $(MAC_OBJS)  \
                $(HAL_OBJS)  \
		$(HDD_OBJS)  \
		$(NLINK_OBJS) \
		$(BTC_OBJS) \
		$(PTT_OBJS)
else

obj-m += libra.o

libra-objs := 	$(VOSS_OBJS) \
                $(TL_OBJS)   \
                $(SSC_OBJS)  \
                $(SME_OBJS)  \
                $(SAP_OBJS)  \
		$(SYS_OBJS)  \
                $(MAC_OBJS)  \
                $(HAL_OBJS)  \
		$(HDD_OBJS)  \
		$(NLINK_OBJS) \
		$(BTC_OBJS) \
		$(PTT_OBJS)
endif
all:
	$(MAKE) $(MAKE_FLAGS) M=`pwd` modules

clean:
	$(MAKE) $(MAKE_FLAGS) M=`pwd` clean
	rm -rf modules.order
	find ../.. -name *.o.cmd  -exec rm {} \; -print
	find ../.. -name *.o  -exec rm {} \; -print
