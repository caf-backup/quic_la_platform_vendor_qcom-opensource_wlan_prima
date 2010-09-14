

#Build/Package Module only in case of 7x27 and msm7630_fusion target
ifneq (, $(filter msm7627_surf msm7627_ffa msm7627_7x_ffa msm7627_7x_surf msm7630_fusion, $(TARGET_PRODUCT)))
        include vendor/qcom/proprietary/wlan/CORE/HDD/src/Android.mk
        include vendor/qcom/proprietary/wlan/utils/ptt/Android.mk
        include vendor/qcom/proprietary/wlan/utils/asf/src/Android.mk
endif


#Build/Package Libra and Volans Module only in case of msm7x30_surf target
ifneq (, $(filter msm7630_surf, $(TARGET_PRODUCT)))
        include vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/Android.mk
        include vendor/qcom/proprietary/wlan/libra/CORE/HDD/src/Android.mk
        include vendor/qcom/proprietary/wlan/libra/utils/ptt/Android.mk
        include vendor/qcom/proprietary/wlan/libra/utils/asf/src/Android.mk
endif

#Build/Package Volans Module only in case of msm8660_surf target
ifneq (, $(filter msm8660_surf, $(TARGET_PRODUCT)))
        include vendor/qcom/proprietary/wlan/volans/CORE/HDD/src/Android.mk
endif
