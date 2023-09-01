# -*- coding: utf-8 -*-
"""
@File        : get_tosun_devicesinfo.py
@Author      : quan.sun@jiduauto.com
@Time        : 2023/08/23 8:36 AM
@Description : description about this file
@Examples    : example of how to use it
"""

from libTSCANAPI import *


def get_device_info(DeviceCount: int = 0):
    AFManufacturer = c_char_p()
    AFProduct = c_char_p()
    AFSerial = c_char_p()
    tscan_get_device_info(
        DeviceCount, AFProduct, AFSerial, AFSerial
    )  # DeviceCount 获取设备信息(选择要连接的设备   0)

    return (
        AFManufacturer.value,
        AFProduct.value,
        AFSerial.value,
    )


if __name__ == "__main__":
    # worke dir: ecu_simulator/
    initialize_lib_tscan(True, True, False)  # 函数初始化

    # scan tosun 设备
    ADeviceScan = s32(0)
    tscan_scan_devices(ADeviceScan)

    print(f"FR current_device num is {ADeviceScan.value}")
    # ADeviceScan = s32(1)
    if ADeviceScan.value > 0:
        for i in range(ADeviceScan.value):
            manufacturer, product, serial = get_device_info(i)
            print(
                f"FR 设备信息 {i+1}： manufacturer is {manufacturer}, product is {product}, serial is {serial}"
            )
    else:
        print("没有发现 FR 设备")

    finalize_lib_tscan()
