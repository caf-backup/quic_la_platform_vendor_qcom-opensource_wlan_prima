
#Build/Package Libra and Volans Module only in case of msm7x30_surf target
ifneq (, $(filter msm7627_surf msm7627_ffa msm7630_surf, $(QCOM_TARGET_PRODUCT)))
#        include vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/Android.mk
        include vendor/qcom/proprietary/wlan/libra/CORE/HDD/src/Android.mk
        include vendor/qcom/proprietary/wlan/libra/utils/ptt/Android.mk
        include vendor/qcom/proprietary/wlan/libra/utils/asf/src/Android.mk
endif

#Build/Package Volans Module only in case of msm8660 target
#ifneq (, $(filter msm8660_surf msm8660_csfb, $(QCOM_TARGET_PRODUCT)))
#        include vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/Android.mk
#endif
