ATH_BLD_DIR := $(call my-dir)

#include $(ATH_BLD_DIR)/ath6kl-fwlog/Android.mk
include $(ATH_BLD_DIR)/ath6kl_fw/Android.mk
include $(ATH_BLD_DIR)/libtcmd/Android.mk
include $(ATH_BLD_DIR)/ath6kl-tcmd/Android.mk
include $(ATH_BLD_DIR)/artagent/Android.mk
include $(ATH_BLD_DIR)/btfilter/Android.mk
