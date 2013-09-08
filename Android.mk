WLAN_BLD_DIR := $(call my-dir)

ifeq ($(BOARD_HAS_QCOM_WLAN), true)
#Build/Package Libra Mono only in case of 7627 target
#ifeq ($(call is-chipset-in-board-platform,msm7627),true)
#        include $(WLAN_BLD_DIR)/libra/CORE/HDD/src/Android.mk
#        include $(WLAN_BLD_DIR)/utils/ptt/Android.mk
#        include $(WLAN_BLD_DIR)/utils/asf/src/Android.mk
#endif

#Build/Package Libra and Volans Module in case of msm7x30_surf target
ifeq ($(call is-board-platform,msm7630_surf),true)
        include $(WLAN_BLD_DIR)/volans/CORE/HDD/src/Android.mk
#       include $(WLAN_BLD_DIR)/libra/CORE/HDD/src/Android.mk
#        include $(WLAN_BLD_DIR)/utils/ptt/Android.mk
#        include $(WLAN_BLD_DIR)/utils/asf/src/Android.mk
endif

#Build/Package Volans Module only in case of 8660 target variants
ifeq ($(call is-board-platform,msm8660),true)
        include $(WLAN_BLD_DIR)/volans/CORE/HDD/src/Android.mk
#        include $(WLAN_BLD_DIR)/utils/ptt/Android.mk
#        include $(WLAN_BLD_DIR)/utils/asf/src/Android.mk
endif

#Build/Package Volans Module only in case of 7627a target
ifeq ($(call is-board-platform,msm7627a),true)
        include $(WLAN_BLD_DIR)/volans/CORE/HDD/src/Android.mk
#        include $(WLAN_BLD_DIR)/utils/ptt/Android.mk
#        include $(WLAN_BLD_DIR)/utils/asf/src/Android.mk
endif

#Build/Package Prima/Pronto Module in case of A & B family targets
ifeq ($(BOARD_WLAN_DEVICE),qcwcn)
        include $(WLAN_BLD_DIR)/prima/Android.mk
endif

endif

