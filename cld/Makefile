MODNAME := wlan
WLAN_ROOT := $(shell pwd)

# These are configurable via Kconfig for kernel-based builds
# Need to explicitly configure for Android-based builds

#Flag to enable BlueTooth AMP feature
CONFIG_PRIMA_WLAN_BTAMP := n

#Flag to enable Legacy Fast Roaming(LFR)
CONFIG_PRIMA_WLAN_LFR := y

#JB kernel has PMKSA patches, hence enabling this flag
CONFIG_PRIMA_WLAN_OKC := y

# JB kernel has CPU enablement patches, so enable
#CONFIG_PRIMA_WLAN_11AC_HIGH_TP := y

#Flag to enable TDLS feature
CONFIG_QCOM_TDLS := n

# Feature flags which are not (currently) configurable via Kconfig

#Whether to build debug version
BUILD_DEBUG_VERSION := 1

#Enable this flag to build driver in diag version
BUILD_DIAG_VERSION := 0

#Do we panic on bug?  default is to warn
PANIC_ON_BUG := 0

#Re-enable wifi on WDI timeout
RE_ENABLE_WIFI_ON_WDI_TIMEOUT := 0

#Enable use of VOS PKT Framework
CONFIG_USE_VOS_PKT_FRAMEWORK := 0

#Enable OS specific ADF abstraction
CONFIG_ADF_SUPPORT := 1

#Enable OL debug and wmi unified functions
CONFIG_ATH_PERF_PWR_OFFLOAD := 1

#Disable packet log
CONFIG_REMOVE_PKT_LOG := 1

#Enable 11AC TX
CONFIG_ATH_11AC_TXCOMPACT := 1

#Enable OS specific IRQ abstraction
CONFIG_ATH_SUPPORT_SHARED_IRQ := 1

#Enable message based HIF instead of RAW access in BMI
CONFIG_HIF_MESSAGE_BASED := 1

#Enable PCI specific APIS (dma, etc)
CONFIG_HIF_PCI := 1

#Enable pci read/write config functions
CONFIG_ATH_PCI := 1

#Enable power management suspend/resume functionality to PCI
CONFIG_ATH_BUS_PM := 1

#Enable/Disable WLAN Integrated SOC
CONFIG_FEATURE_WLAN_INTEGRATED_SOC := 0

#Remove TL
CONFIG_REMOVE_TL := 1

#Enable dword alignment for IP header
CONFIG_IP_HDR_ALIGNMENT := 0

#Enable FLOWMAC module support
CONFIG_ATH_SUPPORT_FLOWMAC_MODULE := 0

#Enable spectral support
CONFIG_ATH_SUPPORT_SPECTRAL := 0

#Enable IBSS HT support
CONFIG_ATH_SUPPORT_IBSS_HT := 0

#Enable HOST TX Sched Debugging
CONFIG_HOST_TX_SCHED_DEBUG := 0

#Enable HOST statistics support
CONFIG_SUPPORT_HOST_STATISTICS := 0

#Enable compilation of ongoing development code
CONFIG_NOT_YET := 0

#Enable RX Reorder timeout support
CONFIG_QCA_SUPPORT_OL_RX_REORDER_TIMEOUT := 0

#Enable WDI Event support
CONFIG_WDI_EVENT_ENABLE := 0

#Endianess selection
CONFIG_LITTLE_ENDIAN := 1

#Enable HTT Data debug
CONFIG_HTT_DATA_DEBUG := 0

#Enable checksum offload
CONFIG_CHECKSUM_OFFLOAD := 0

#Enable TX reclaim support
CONFIG_TX_CREDIT_RECLAIM_SUPPORT := 0

HAVE_CFG80211 := 1
EXTRA_CFLAGS += -DCONFIG_CFG80211=1

############ COMMON ############
COMMON_DIR := CORE/COMMON
COMMON_INC := -I$(WLAN_ROOT)/$(COMMON_DIR)

########### adf ###########
ADF_DIR := $(COMMON_DIR)/adf
ADF_INC_DIR := $(COMMON_DIR)/adf

ADF_INC := -I$(WLAN_ROOT)/$(ADF_INC_DIR) \
	   -I$(WLAN_ROOT)/$(ADF_INC_DIR)/linux \
	   -I$(WLAN_ROOT)/$(COMMON_DIR)/asf \
	   -I$(WLAN_ROOT)/$(COMMON_DIR)/adf

ADF_OBJS := $(ADF_DIR)/adf_nbuf.o \
	    $(ADF_DIR)/adf_os_lock.o \
	    $(ADF_DIR)/adf_os_mem.o \
	    $(ADF_DIR)/linux/adf_os_lock_pvt.o

############ BAP ############
BAP_DIR :=	CORE/BAP
BAP_INC_DIR :=	$(BAP_DIR)/inc
BAP_SRC_DIR :=	$(BAP_DIR)/src

BAP_INC := 	-I$(WLAN_ROOT)/$(BAP_INC_DIR) \
		-I$(WLAN_ROOT)/$(BAP_SRC_DIR)

BAP_OBJS := 	$(BAP_SRC_DIR)/bapApiData.o \
		$(BAP_SRC_DIR)/bapApiDebug.o \
		$(BAP_SRC_DIR)/bapApiExt.o \
		$(BAP_SRC_DIR)/bapApiHCBB.o \
		$(BAP_SRC_DIR)/bapApiInfo.o \
		$(BAP_SRC_DIR)/bapApiLinkCntl.o \
		$(BAP_SRC_DIR)/bapApiLinkSupervision.o \
		$(BAP_SRC_DIR)/bapApiStatus.o \
		$(BAP_SRC_DIR)/bapApiTimer.o \
		$(BAP_SRC_DIR)/bapModule.o \
		$(BAP_SRC_DIR)/bapRsn8021xAuthFsm.o \
		$(BAP_SRC_DIR)/bapRsn8021xPrf.o \
		$(BAP_SRC_DIR)/bapRsn8021xSuppRsnFsm.o \
		$(BAP_SRC_DIR)/bapRsnAsfPacket.o \
		$(BAP_SRC_DIR)/bapRsnSsmAesKeyWrap.o \
		$(BAP_SRC_DIR)/bapRsnSsmEapol.o \
		$(BAP_SRC_DIR)/bapRsnSsmReplayCtr.o \
		$(BAP_SRC_DIR)/bapRsnTxRx.o \
		$(BAP_SRC_DIR)/btampFsm.o \
		$(BAP_SRC_DIR)/btampHCI.o

############ DXE ############
DXE_DIR :=	CORE/DXE
DXE_INC_DIR :=	$(DXE_DIR)/inc
DXE_SRC_DIR :=	$(DXE_DIR)/src

DXE_INC := 	-I$(WLAN_ROOT)/$(DXE_INC_DIR) \
		-I$(WLAN_ROOT)/$(DXE_SRC_DIR)

DXE_OBJS = 	$(DXE_SRC_DIR)/wlan_qct_dxe.o \
		$(DXE_SRC_DIR)/wlan_qct_dxe_cfg_i.o

############ HDD ############
HDD_DIR :=	CORE/HDD
HDD_INC_DIR :=	$(HDD_DIR)/inc
HDD_SRC_DIR :=	$(HDD_DIR)/src

HDD_INC := 	-I$(WLAN_ROOT)/$(HDD_INC_DIR) \
		-I$(WLAN_ROOT)/$(HDD_SRC_DIR)

HDD_OBJS :=	$(HDD_SRC_DIR)/wlan_hdd_assoc.o \
		$(HDD_SRC_DIR)/wlan_hdd_cfg.o \
		$(HDD_SRC_DIR)/wlan_hdd_dev_pwr.o \
		$(HDD_SRC_DIR)/wlan_hdd_dp_utils.o \
		$(HDD_SRC_DIR)/wlan_hdd_early_suspend.o \
		$(HDD_SRC_DIR)/wlan_hdd_ftm.o \
		$(HDD_SRC_DIR)/wlan_hdd_hostapd.o \
		$(HDD_SRC_DIR)/wlan_hdd_oemdata.o \
		$(HDD_SRC_DIR)/wlan_hdd_main.o \
		$(HDD_SRC_DIR)/wlan_hdd_mib.o \
		$(HDD_SRC_DIR)/wlan_hdd_scan.o \
		$(HDD_SRC_DIR)/wlan_hdd_softap_tx_rx.o \
		$(HDD_SRC_DIR)/wlan_hdd_tx_rx.o \
		$(HDD_SRC_DIR)/wlan_hdd_wext.o \
		$(HDD_SRC_DIR)/wlan_hdd_wmm.o \
		$(HDD_SRC_DIR)/wlan_hdd_wowl.o

ifeq ($(HAVE_CFG80211),1)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_cfg80211.o \
		$(HDD_SRC_DIR)/wlan_hdd_p2p.o
endif

ifeq ($(CONFIG_QCOM_TDLS),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_tdls.o
endif

############ MAC ############
MAC_DIR :=	CORE/MAC
MAC_INC_DIR :=	$(MAC_DIR)/inc
MAC_SRC_DIR :=	$(MAC_DIR)/src

MAC_INC := 	-I$(WLAN_ROOT)/$(MAC_INC_DIR) \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/dph \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/include \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/pe/include \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/pe/lim

MAC_CFG_OBJS := $(MAC_SRC_DIR)/cfg/cfgApi.o \
		$(MAC_SRC_DIR)/cfg/cfgDebug.o \
		$(MAC_SRC_DIR)/cfg/cfgParamName.o \
		$(MAC_SRC_DIR)/cfg/cfgProcMsg.o \
		$(MAC_SRC_DIR)/cfg/cfgSendMsg.o

MAC_DPH_OBJS :=	$(MAC_SRC_DIR)/dph/dphHashTable.o

MAC_LIM_OBJS := $(MAC_SRC_DIR)/pe/lim/limAIDmgmt.o \
		$(MAC_SRC_DIR)/pe/lim/limAdmitControl.o \
		$(MAC_SRC_DIR)/pe/lim/limApi.o \
		$(MAC_SRC_DIR)/pe/lim/limAssocUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limDebug.o \
		$(MAC_SRC_DIR)/pe/lim/limFT.o \
		$(MAC_SRC_DIR)/pe/lim/limIbssPeerMgmt.o \
		$(MAC_SRC_DIR)/pe/lim/limLinkMonitoringAlgo.o \
		$(MAC_SRC_DIR)/pe/lim/limLogDump.o \
		$(MAC_SRC_DIR)/pe/lim/limP2P.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessActionFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessAssocReqFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessAssocRspFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessAuthFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessBeaconFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessCfgUpdates.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessDeauthFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessDisassocFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessLmmMessages.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessMessageQueue.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessMlmReqMessages.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessMlmRspMessages.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessProbeReqFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessProbeRspFrame.o \
		$(MAC_SRC_DIR)/pe/lim/limProcessSmeReqMessages.o \
		$(MAC_SRC_DIR)/pe/lim/limPropExtsUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limRoamingAlgo.o \
		$(MAC_SRC_DIR)/pe/lim/limScanResultUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limSecurityUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limSendManagementFrames.o \
		$(MAC_SRC_DIR)/pe/lim/limSendMessages.o \
		$(MAC_SRC_DIR)/pe/lim/limSendSmeRspMessages.o \
		$(MAC_SRC_DIR)/pe/lim/limSerDesUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limSession.o \
		$(MAC_SRC_DIR)/pe/lim/limSessionUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limSmeReqUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limStaHashApi.o \
		$(MAC_SRC_DIR)/pe/lim/limTimerUtils.o \
		$(MAC_SRC_DIR)/pe/lim/limTrace.o \
		$(MAC_SRC_DIR)/pe/lim/limUtils.o

ifeq ($(CONFIG_QCOM_CCX),y)
MAC_LIM_OBJS += $(MAC_SRC_DIR)/pe/lim/limProcessCcxFrame.o
endif

ifeq ($(CONFIG_QCOM_TDLS),y)
MAC_LIM_OBJS += $(MAC_SRC_DIR)/pe/lim/limProcessTdls.o
endif

MAC_PMM_OBJS := $(MAC_SRC_DIR)/pe/pmm/pmmAP.o \
		$(MAC_SRC_DIR)/pe/pmm/pmmApi.o \
		$(MAC_SRC_DIR)/pe/pmm/pmmDebug.o

MAC_SCH_OBJS := $(MAC_SRC_DIR)/pe/sch/schApi.o \
		$(MAC_SRC_DIR)/pe/sch/schBeaconGen.o \
		$(MAC_SRC_DIR)/pe/sch/schBeaconProcess.o \
		$(MAC_SRC_DIR)/pe/sch/schDebug.o \
		$(MAC_SRC_DIR)/pe/sch/schMessage.o

MAC_RRM_OBJS :=	$(MAC_SRC_DIR)/pe/rrm/rrmApi.o

MAC_OBJS := 	$(MAC_CFG_OBJS) \
		$(MAC_DPH_OBJS) \
		$(MAC_LIM_OBJS) \
		$(MAC_PMM_OBJS) \
		$(MAC_SCH_OBJS) \
		$(MAC_RRM_OBJS)

############ SAP ############
SAP_DIR :=	CORE/SAP
SAP_INC_DIR :=	$(SAP_DIR)/inc
SAP_SRC_DIR :=	$(SAP_DIR)/src

SAP_INC := 	-I$(WLAN_ROOT)/$(SAP_INC_DIR) \
		-I$(WLAN_ROOT)/$(SAP_SRC_DIR)

SAP_OBJS :=	$(SAP_SRC_DIR)/sapApiLinkCntl.o \
		$(SAP_SRC_DIR)/sapChSelect.o \
		$(SAP_SRC_DIR)/sapFsm.o \
		$(SAP_SRC_DIR)/sapModule.o

############ SME ############
SME_DIR :=	CORE/SME
SME_INC_DIR :=	$(SME_DIR)/inc
SME_SRC_DIR :=	$(SME_DIR)/src

SME_INC := 	-I$(WLAN_ROOT)/$(SME_INC_DIR) \
		-I$(WLAN_ROOT)/$(SME_SRC_DIR)/csr

SME_CCM_OBJS := $(SME_SRC_DIR)/ccm/ccmApi.o \
		$(SME_SRC_DIR)/ccm/ccmLogDump.o

SME_CSR_OBJS := $(SME_SRC_DIR)/csr/csrApiRoam.o \
		$(SME_SRC_DIR)/csr/csrApiScan.o \
		$(SME_SRC_DIR)/csr/csrCmdProcess.o \
		$(SME_SRC_DIR)/csr/csrLinkList.o \
		$(SME_SRC_DIR)/csr/csrLogDump.o \
		$(SME_SRC_DIR)/csr/csrNeighborRoam.o \
		$(SME_SRC_DIR)/csr/csrUtil.o

ifeq ($(CONFIG_QCOM_CCX),y)
SME_CSR_OBJS += $(SME_SRC_DIR)/csr/csrCcx.o
endif

ifeq ($(CONFIG_QCOM_TDLS),y)
SME_CSR_OBJS += $(SME_SRC_DIR)/csr/csrTdlsProcess.o
endif

SME_PMC_OBJS := $(SME_SRC_DIR)/pmc/pmcApi.o \
		$(SME_SRC_DIR)/pmc/pmc.o \
		$(SME_SRC_DIR)/pmc/pmcLogDump.o

SME_QOS_OBJS := $(SME_SRC_DIR)/QoS/sme_Qos.o

SME_CMN_OBJS := $(SME_SRC_DIR)/sme_common/sme_Api.o \
		$(SME_SRC_DIR)/sme_common/sme_FTApi.o

SME_BTC_OBJS := $(SME_SRC_DIR)/btc/btcApi.o

SME_OEM_DATA_OBJS := $(SME_SRC_DIR)/oemData/oemDataApi.o

SME_P2P_OBJS = $(SME_SRC_DIR)/p2p/p2p_Api.o

SME_RRM_OBJS := $(SME_SRC_DIR)/rrm/sme_rrm.o

SME_OBJS :=	$(SME_BTC_OBJS) \
		$(SME_CCM_OBJS) \
		$(SME_CMN_OBJS) \
		$(SME_CSR_OBJS) \
		$(SME_OEM_DATA_OBJS) \
		$(SME_P2P_OBJS) \
		$(SME_PMC_OBJS) \
		$(SME_QOS_OBJS) \
		$(SME_RRM_OBJS)

############ SVC ############
SVC_DIR :=	CORE/SVC
SVC_INC_DIR :=	$(SVC_DIR)/inc
SVC_SRC_DIR :=	$(SVC_DIR)/src

SVC_INC := 	-I$(WLAN_ROOT)/$(SVC_INC_DIR) \
		-I$(WLAN_ROOT)/$(SVC_DIR)/external

BTC_SRC_DIR :=	$(SVC_SRC_DIR)/btc
BTC_OBJS :=	$(BTC_SRC_DIR)/wlan_btc_svc.o

NLINK_SRC_DIR := $(SVC_SRC_DIR)/nlink
NLINK_OBJS :=	$(NLINK_SRC_DIR)/wlan_nlink_srv.o

PTT_SRC_DIR :=	$(SVC_SRC_DIR)/ptt
PTT_OBJS :=	$(PTT_SRC_DIR)/wlan_ptt_sock_svc.o

SVC_OBJS :=	$(BTC_OBJS) \
		$(NLINK_OBJS) \
		$(PTT_OBJS)

############ SYS ############
SYS_DIR :=	CORE/SYS

SYS_INC := 	-I$(WLAN_ROOT)/$(SYS_DIR)/common/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/pal/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/platform/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/system/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/utils/inc

SYS_COMMON_SRC_DIR := $(SYS_DIR)/common/src
SYS_LEGACY_SRC_DIR := $(SYS_DIR)/legacy/src
SYS_OBJS :=	$(SYS_COMMON_SRC_DIR)/wlan_qct_sys.o \
		$(SYS_LEGACY_SRC_DIR)/pal/src/palApiComm.o \
		$(SYS_LEGACY_SRC_DIR)/pal/src/palTimer.o \
		$(SYS_LEGACY_SRC_DIR)/platform/src/VossWrapper.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/macInitApi.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/sysEntryFunc.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/dot11f.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/logApi.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/logDump.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/macTrace.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/parserApi.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/utilsApi.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/utilsParser.o

ifeq ($(CONFIG_QCOM_CCX),y)
SYS_OBJS += $(SYS_LEGACY_SRC_DIR)/utils/src/limCcxparserApi.o
endif

############ VOSS ############
VOSS_DIR :=	CORE/VOSS
VOSS_INC_DIR :=	$(VOSS_DIR)/inc
VOSS_SRC_DIR :=	$(VOSS_DIR)/src

VOSS_INC := 	-I$(WLAN_ROOT)/$(VOSS_INC_DIR) \
		-I$(WLAN_ROOT)/$(VOSS_SRC_DIR)

VOSS_OBJS :=    $(VOSS_SRC_DIR)/vos_api.o \
		$(VOSS_SRC_DIR)/vos_event.o \
		$(VOSS_SRC_DIR)/vos_getBin.o \
		$(VOSS_SRC_DIR)/vos_list.o \
		$(VOSS_SRC_DIR)/vos_lock.o \
		$(VOSS_SRC_DIR)/vos_memory.o \
		$(VOSS_SRC_DIR)/vos_mq.o \
		$(VOSS_SRC_DIR)/vos_nvitem.o \
		$(VOSS_SRC_DIR)/vos_power.o \
		$(VOSS_SRC_DIR)/vos_sched.o \
		$(VOSS_SRC_DIR)/vos_threads.o \
		$(VOSS_SRC_DIR)/vos_timer.o \
		$(VOSS_SRC_DIR)/vos_trace.o \
		$(VOSS_SRC_DIR)/vos_types.o \
		$(VOSS_SRC_DIR)/vos_utils.o \
		$(VOSS_SRC_DIR)/packet.o

ifeq ($(BUILD_DIAG_VERSION),1)
VOSS_OBJS += $(VOSS_SRC_DIR)/vos_diag.o
endif

########### BMI ###########
BMI_DIR := CORE/BMI

BMI_INC := -I$(WLAN_ROOT)/$(BMI_DIR)

BMI_OBJS := $(BMI_DIR)/bmi.o \
	    $(BMI_DIR)/ol_fw.o

########### WMI ###########
WMI_DIR := CORE/WMI

WMI_INC := -I$(WLAN_ROOT)/$(WMI_DIR)

WMI_OBJS := $(WMI_DIR)/wmi_unified.o

############ TXRX ############
TXRX_DIR :=	CORE/TXRX
TXRX_INC := 	-I$(WLAN_ROOT)/$(TXRX_DIR)

TXRX_OBJS := $(TXRX_DIR)/txrx_api.o \
	     $(TXRX_DIR)/ol_txrx.o \
	     $(TXRX_DIR)/ol_cfg.o \
	     $(TXRX_DIR)/ol_rx.o \
	     $(TXRX_DIR)/ol_rx_fwd.o \
	     $(TXRX_DIR)/ol_txrx.o \
	     $(TXRX_DIR)/ol_rx_defrag.o \
	     $(TXRX_DIR)/ol_tx_desc.o \
	     $(TXRX_DIR)/ol_tx_classify.o \
	     $(TXRX_DIR)/ol_tx.o \
	     $(TXRX_DIR)/ol_rx_reorder_timeout.o \
	     $(TXRX_DIR)/ol_rx_reorder.o \
	     $(TXRX_DIR)/ol_rx_pn.o \
	     $(TXRX_DIR)/ol_tx_queue.o \
	     $(TXRX_DIR)/ol_txrx_peer_find.o \
	     $(TXRX_DIR)/ol_txrx_event.o \
	     $(TXRX_DIR)/ol_txrx_encap.o \
	     $(TXRX_DIR)/ol_tx_send.o \
	     $(TXRX_DIR)/ol_tx_sched.o


############ HTT ############
HTT_DIR :=	CORE/HTT
HTT_INC :=	-I$(WLAN_ROOT)/$(HTT_DIR)

HTT_OBJS := $(HTT_DIR)/htt_tx.o \
	    $(HTT_DIR)/htt.o \
	    $(HTT_DIR)/htt_t2h.o \
	    $(HTT_DIR)/htt_h2t.o \
	    $(HTT_DIR)/htt_fw_stats.o \
	    $(HTT_DIR)/htt_rx.o


########### HTC ###########
HTC_DIR := CORE/HTC

HTC_INC := -I$(WLAN_ROOT)/$(HTC_DIR)

HTC_OBJS := $(HTC_DIR)/htc.o \
	    $(HTC_DIR)/htc_send.o \
	    $(HTC_DIR)/htc_recv.o \
	    $(HTC_DIR)/htc_services.o

########### HIF ###########
HIF_DIR := CORE/HIF

HIF_INC := -I$(WLAN_ROOT)/$(HIF_DIR)

HIF_OBJS := $(HIF_DIR)/copy_engine.o \
	    $(HIF_DIR)/hif_pci.o \
	    $(HIF_DIR)/if_pci.o \
	    $(HIF_DIR)/regtable.o

 ############ WMA ############
WMA_DIR :=	CORE/WMA
WMA_INC_DIR :=	$(WMA_DIR)/inc
 
WMA_INC := 	-I$(WLAN_ROOT)/$(WMA_INC_DIR) \
		-I$(WLAN_ROOT)/$(WMA_DIR) \
 		-I$(WLAN_ROOT)/$(WMA_INC_DIR)/legacy 

WMA_OBJS :=	$(WMA_DIR)/wlan_qct_wma_legacy.o \
		$(WMA_DIR)/wma.o \
		$(WMA_DIR)/wlan_nv.o

############ WDA ############
WDA_DIR :=	CORE/WDA
WDA_INC_DIR :=	$(WDA_DIR)/inc
WDA_SRC_DIR :=	$(WDA_DIR)/src

WDA_INC := 	-I$(WLAN_ROOT)/$(WDA_INC_DIR) \
		-I$(WLAN_ROOT)/$(WDA_INC_DIR)/legacy \
		-I$(WLAN_ROOT)/$(WDA_SRC_DIR)

RIVA_INC :=	-I$(WLAN_ROOT)/riva/inc

LINUX_INC :=	-Iinclude/linux

INCS :=		$(VOSS_INC) \
		$(DXE_INC) \
		$(HDD_INC) \
		$(LINUX_INC) \
		$(MAC_INC) \
		$(RIVA_INC) \
		$(SAP_INC) \
		$(SME_INC) \
		$(SVC_INC) \
		$(SYS_INC) \
		$(WMA_INC) \
		$(ADF_INC) \
		$(COMMON_INC) \
		$(BMI_INC) \
		$(WMI_INC) \
		$(TXRX_INC) \
		$(HTT_INC) \
		$(HTC_INC) \
		$(HIF_INC)

OBJS :=		$(HDD_OBJS) \
		$(MAC_OBJS) \
		$(SAP_OBJS) \
		$(SME_OBJS) \
		$(SVC_OBJS) \
		$(SYS_OBJS) \
		$(VOSS_OBJS) \
		$(ADF_OBJS) \
		$(BMI_OBJS) \
		$(WMI_OBJS) \
		$(TXRX_OBJS) \
		$(HTT_OBJS) \
		$(HTC_OBJS) \
		$(WMA_OBJS) \
		$(HIF_OBJS)

EXTRA_CFLAGS += $(INCS)

CDEFINES :=	-DANI_PRODUCT_TYPE_CLIENT=1 \
		-DANI_LITTLE_BYTE_ENDIAN \
		-DANI_LITTLE_BIT_ENDIAN \
		-DWLAN_STA=1 \
		-DAP=2 \
		-DWNI_POLARIS_FW_PRODUCT=1 \
		-DQC_WLAN_CHIPSET_PRIMA \
		-DINTEGRATION_READY \
		-DVOSS_ENABLED \
		-DDOT11F_LITTLE_ENDIAN_HOST \
		-DGEN6_ONWARDS \
		-DANI_COMPILER_TYPE_GCC \
		-DANI_OS_TYPE_ANDROID=6 \
		-DADVANCED=3 \
		-DWNI_POLARIS_FW_PACKAGE=9 \
		-DANI_LOGDUMP \
		-DWLAN_PERF \
		-DUSE_LOCKED_QUEUE \
		-DPTT_SOCK_SVC_ENABLE \
		-DFEATURE_WLAN_UAPSD_FW_TRG_FRAMES \
		-DWLAN_SOFTAP_FEATURE \
		-Wall\
		-D__linux__ \
		-DHAL_SELF_STA_PER_BSS=1 \
		-DANI_MANF_DIAG \
		-DWLAN_FEATURE_VOWIFI_11R \
		-DWLAN_FEATURE_NEIGHBOR_ROAMING \
		-DWLAN_FEATURE_NEIGHBOR_ROAMING_DEBUG \
		-DWLAN_FEATURE_VOWIFI_11R_DEBUG \
		-DFEATURE_OEM_DATA_SUPPORT\
		-DSOFTAP_CHANNEL_RANGE \
		-DWLAN_AP_STA_CONCURRENCY \
		-DFEATURE_WLAN_SCAN_PNO \
		-DWLAN_FEATURE_PACKET_FILTERING \
		-DWLAN_FEATURE_VOWIFI \
		-DWLAN_FEATURE_11AC \
		-DWLAN_FEATURE_P2P_DEBUG \
		-DWLAN_ENABLE_AGEIE_ON_SCAN_RESULTS \
		-DWLAN_ACTIVEMODE_OFFLOAD_FEATURE \
		-DWLAN_SOFTAP_VSTA_FEATURE \
		-DREMOVE_TL \
		-DWMA_LAYER \
		-DADF_SUPPORT

ifneq ($(CONFIG_PRONTO_WLAN),)
CDEFINES += -DWCN_PRONTO
CDEFINES += -DWCN_PRONTO_V1
endif

ifeq ($(BUILD_DEBUG_VERSION),1)
CDEFINES +=	-DWLAN_DEBUG \
		-DTRACE_RECORD \
		-DPE_DEBUG_LOGW \
		-DPE_DEBUG_LOGE \
		-DDEBUG
endif

ifeq ($(CONFIG_SLUB_DEBUG_ON),y)
CDEFINES += -DTIMER_MANAGER
CDEFINES += -DMEMORY_DEBUG
endif

ifeq ($(HAVE_CFG80211),1)
CDEFINES += -DWLAN_FEATURE_P2P
CDEFINES += -DWLAN_FEATURE_WFD
endif

ifeq ($(CONFIG_QCOM_CCX),y)
CDEFINES += -DFEATURE_WLAN_CCX
endif

#normally, TDLS negative behavior is not needed
ifeq ($(CONFIG_QCOM_TDLS),y)
CDEFINES += -DFEATURE_WLAN_TDLS
#CDEFINES += -DFEATURE_WLAN_TDLS_NEGATIVE
#Code under FEATURE_WLAN_TDLS_INTERNAL is ported from volans, This code
#is not tested only verifed that it compiles. This is not required for
#supplicant based implementation
#CDEFINES += -DFEATURE_WLAN_TDLS_INTERNAL
endif

ifeq ($(CONFIG_PRIMA_WLAN_BTAMP),y)
CDEFINES += -DWLAN_BTAMP_FEATURE
endif

ifeq ($(CONFIG_PRIMA_WLAN_LFR),y)
CDEFINES += -DFEATURE_WLAN_LFR
endif

ifeq ($(CONFIG_PRIMA_WLAN_OKC),y)
CDEFINES += -DFEATURE_WLAN_OKC
endif

ifeq ($(CONFIG_PRIMA_WLAN_11AC_HIGH_TP),y)
CDEFINES += -DWLAN_FEATURE_11AC_HIGH_TP
endif

ifeq ($(BUILD_DIAG_VERSION),1)
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT_CSR
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT_LIM
endif

# enable the MAC Address auto-generation feature
CDEFINES += -DWLAN_AUTOGEN_MACADDR_FEATURE

ifeq ($(PANIC_ON_BUG),1)
CDEFINES += -DPANIC_ON_BUG
endif

ifeq ($(RE_ENABLE_WIFI_ON_WDI_TIMEOUT),1)
CDEFINES += -DWDI_RE_ENABLE_WIFI_ON_WDI_TIMEOUT
endif

ifeq ($(KERNEL_BUILD),1)
CDEFINES += -DWLAN_OPEN_SOURCE
endif

#Enable use of VOS PKT Framework
ifeq ($(CONFIG_USE_VOS_PKT_FRAMEWORK), 1)
CDEFINES += -DUSE_VOS_PKT_FRAMEWORK
endif

#Enable the OS specific ADF abstraction
ifeq ($(CONFIG_ADF_SUPPORT), 1)
CDEFINES += -DADF_SUPPORT
endif

#Enable OL debug and wmi unified functions
ifeq ($(CONFIG_ATH_PERF_PWR_OFFLOAD), 1)
CDEFINES += -DATH_PERF_PWR_OFFLOAD
endif

#Disable packet log
ifeq ($(CONFIG_REMOVE_PKT_LOG), 1)
CDEFINES += -DREMOVE_PKT_LOG
endif

#Enable 11AC TX
ifeq ($(CONFIG_ATH_11AC_TXCOMPACT), 1)
CDEFINES += -DATH_11AC_TXCOMPACT
endif

#Enable OS specific IRQ abstraction
ifeq ($(CONFIG_ATH_SUPPORT_SHARED_IRQ), 1)
CDEFINES += -DATH_SUPPORT_SHARED_IRQ
endif

#Enable message based HIF instead of RAW access in BMI
ifeq ($(CONFIG_HIF_MESSAGE_BASED), 1)
CDEFINES += -DHIF_MESSAGE_BASED
endif

#Enable PCI specific APIS (dma, etc)
ifeq ($(CONFIG_HIF_PCI), 1)
CDEFINES += -DHIF_PCI
endif

#Enable pci read/write config functions
ifeq ($(CONFIG_ATH_PCI), 1)
CDEFINES += -DATH_PCI
endif

#Enable power management suspend/resume functionality to PCI
ifeq ($(CONFIG_ATH_BUS_PM), 1)
CDEFINES += -DATH_BUS_PM
endif

#Enable/Disable WLAN Integrated SOC
ifeq ($(CONFIG_FEATURE_WLAN_INTEGRATED_SOC), 1)
CDEFINES += -DFEATURE_WLAN_INTEGRATED_SOC
CDEFINES += -DQCA_SUPPORT_HOST_ADDBA
endif

#Remove TL
ifeq ($(CONFIG_REMOVE_TL), 1)
CDEFINES += -DREMOVE_TL
endif

#Enable dword alignment for IP header
ifeq ($(CONFIG_IP_HDR_ALIGNMENT), 1)
CDEFINES += -DPERE_IP_HDR_ALIGNMENT_WAR
endif

#Enable FLOWMAC module support
ifeq ($(CONFIG_ATH_SUPPORT_FLOWMAC_MODULE), 1)
CDEFINES += -DATH_SUPPORT_FLOWMAC_MODULE
endif

#Enable spectral support
ifeq ($(CONFIG_ATH_SUPPORT_SPECTRAL), 1)
CDEFINES += -DATH_SUPPORT_SPECTRAL
endif

#Enable IBSS HT support
ifeq ($(CONFIG_ATH_SUPPORT_IBSS_HT), 1)
CDEFINES += -DATH_SUPPORT_IBSS_HT
endif

#Enable HOST TX Sched Debugging
ifeq ($(CONFIG_HOST_TX_SCHED_DEBUG), 1)
CDEFINES += -DHOST_TX_SCHED_DEBUG
endif

#Enable HOST statistics support
ifeq ($(CONFIG_SUPPORT_HOST_STATISTICS), 1)
CDEFINES += -DSUPPORT_HOST_STATISTICS
endif

#Enable compilation of ongoing development code
ifeq ($(CONFIG_NOT_YET), 1)
CDEFINES += -DNOT_YET
endif

#Enable RX Reorder timeout support
ifeq ($(CONFIG_QCA_SUPPORT_OL_RX_REORDER_TIMEOUT), 1)
CDEFINES += -DQCA_SUPPORT_OL_RX_REORDER_TIMEOUT
endif
#Enable WDI Event support
ifeq ($(CONFIG_WDI_EVENT_ENABLE), 1)
CDEFINES += -DWDI_EVENT_ENABLE
endif

#Endianess selection
ifeq ($(CONFIG_LITTLE_ENDIAN), 1)
AH_LITTLE_ENDIAN=1234
CDEFINES += -DAH_BYTE_ORDER=$(AH_LITTLE_ENDIAN)
else
AH_BIG_ENDIAN=4321
CDEFINES += -DAH_BYTE_ORDER=$(AH_BIG_ENDIAN)
CDEFINES += -DBIG_ENDIAN_HOST
endif

#Enable HTT Data debug
ifeq ($(CONFIG_HTT_DATA_DEBUG), 1)
CDEFINES += -DHTT_DEBUG_DATA
endif

#Enable checksum offload
ifeq ($(CONFIG_CHECKSUM_OFFLOAD), 1)
CDEFINES += -DCHECKSUM_OFFLOAD
endif

#Enable TX reclaim support
ifeq ($(CONFIG_TX_CREDIT_RECLAIM_SUPPORT), 1)
CDEFINES += -DTX_CREDIT_RECLAIM_SUPPORT
endif

#Enable TX sinlge completions
ifeq ($(CONFIG_QCA_TX_SINGLE_COMPLETIONS), 1)
CDEFINES += -DQCA_TX_SINGLE_COMPLETIONS
endif

#Enable TX standard path only
ifeq ($(CONFIG_QCA_TX_STD_PATH_ONLY), 1)
CDEFINES += -DQCA_TX_STD_PATH_ONLY
endif

#KBUILD_CPPFLAGS += $(CDEFINES)
EXTRA_CFLAGS += $(CDEFINES)

# Module information used by KBuild framework
#obj-$(CONFIG_PRIMA_WLAN) += $(MODNAME).o
#obj-$(CONFIG_PRONTO_WLAN) += $(MODNAME).o
#$(MODNAME)-y := $(OBJS)

obj-m := $(MODNAME).o
wlan-objs := $(OBJS)

KERN_DIR ?= /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)

all:
	$(MAKE) -C $(KERN_DIR) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" M=$(PWD) modules

clean:
	$(MAKE) -C $(KERN_DIR) M=$(PWD) clean
