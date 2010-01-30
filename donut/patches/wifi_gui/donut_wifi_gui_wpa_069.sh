pushd .
cd frameworks/base/
patch -p1 < ../../vendor/qcom-proprietary/wlan/donut/patches/wifi_gui/donut_wifi_gui_frameworks.patch
popd

pushd .
cd hardware/libhardware_legacy/
patch -p1 < ../../vendor/qcom-proprietary/wlan/donut/patches/wifi_gui/donut_wifi_gui_hardware.patch
popd

pushd .
cd system/core/
patch -p1 < ../../vendor/qcom-proprietary/wlan/donut/patches/wifi_gui/donut_wifi_gui_system.patch
popd

