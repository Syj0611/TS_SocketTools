# -*- coding: utf-8 -*-
"""
@File        : bus_app.py
@Author      : quan.sun@jiduatuo.com
@Time        : 2022/10/16 10:51 AM
@Description : simulator of can or lin or fr (cyc,single)
@Examples    : example of how to use it
"""

from cmath import log
import os
from re import L
import sys
import locale


current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(current_path)
sys.path.append(os.path.join(current_path, ".."))
sys.path.append(os.path.join(current_path, "../.."))
sys.path.append(os.path.join(current_path, "../../.."))
import can
from threading import Thread
import threading


import time
from time import sleep


# =========== Convenient and fast manual test  ====================================
exit_flag = False
timeout = 100


def send_single_data_directly(can_id, data, channel, bitrate=500000, is_fd=False):
    # Convenient and fast manual test
    can.rc['interface'] = 'socketcan'
    can.rc['channel'] = channel
    can.rc['bitrate'] = bitrate
    can.rc['fd'] = is_fd
    bus = can.interface.Bus()
    msg = can.Message(
        arbitration_id=can_id, data=data, is_extended_id=False, is_fd=is_fd
    )
    bus.send(msg)
    bus.shutdown()


def add_cyclic_msg_with_data(
        can_id, data, cyclic_rate, channel, bitrate=500000, is_fd=False
):
    # Convenient and fast manual test
    """
    Add cyclic message with specific data to bus
    :param can_id: can message id
    :param data: data of can cyclic message
    :param cyclic_rate: Cycle rate of new cyclic message (in seconds)
    """
    can.rc['interface'] = 'socketcan'
    can.rc['channel'] = channel
    can.rc['bitrate'] = bitrate
    can.rc['fd'] = is_fd
    bus = can.interface.Bus()
    msg = can.Message(
        arbitration_id=can_id, data=data, is_extended_id=False, is_fd=is_fd
    )
    while exit_flag is False:
        try:
            bus.send(msg)
        except:
            pass
        sleep(cyclic_rate)
    bus.shutdown()


def add_cyclic_msg_with_data_run(
        can_id, data, cyclic_rate, channel, bitrate=500000, is_fd=False
):
    cyclic_msg = Thread(
        target=add_cyclic_msg_with_data,
        args=(can_id, data, cyclic_rate, channel, bitrate, is_fd),
    )
    cyclic_msg.start()


def reset_cyclic_msg_flag():
    global exit_flag
    exit_flag = False


def close_cyclic_msg():
    global exit_flag
    exit_flag = True


def receives(func, channel, wait=100, bustype='socketcan', bitrate=500000):
    global timeout
    timeout = wait
    rx_bus = can.interface.Bus(bustype=bustype, channel=channel, bitrate=bitrate)
    notifier = can.Notifier(rx_bus, [func])

    sleep(timeout)
    # Close Ecu_Sim by stopping notifier and interface_bus
    try:
        notifier.stop()
        rx_bus.socket.close()
        rx_bus.shutdown()
        
    except AttributeError:
        print('bus close error: {}'.format(channel))


def rx_msg(msg):
    print("msg.arbitration_id is {}".format(msg.arbitration_id))
    print("msg is {}".format(msg))


# ================================================================================================================
if __name__ =='__main__':
    add_cyclic_msg_with_data_run(1,[1,2,3],0.1,'can0',is_fd=True)
    from libTSCANAPI import *


    def on_can_event(obj,ACANFD):
        if (ACANFD.contents.FProperties&1)==1:
            print('tx')
        else:
            print('rx')

    ONCAN = OnTx_RxFUNC_CANFD_WHandle(on_can_event)

    i = 0
    while True:
        i+=1
        print(f"第 {i} 次")
        hwHandle = size_t(0)
        initialize_lib_tscan(True,True,False)
        ret = tsapp_connect(b"",hwHandle)
        if ret != 0 and ret !=5:
            break

        # tsapp_register_event_canfd_whandle(hwHandle,ONCAN)

        # 波特率配置
        """
        0:  通道
        500: 仲裁段波特率
        2000: 数据段波特率
        最后一个参数为 是否激活终端电阻
        """



        tsapp_configure_baudrate_canfd(hwHandle,0,500,2000,TLIBCANFDControllerType.lfdtISOCAN,TLIBCANFDControllerMode.lfdmNormal,True)
        tsapp_configure_baudrate_canfd(hwHandle,1,500,2000,TLIBCANFDControllerType.lfdtISOCAN,TLIBCANFDControllerMode.lfdmNormal,True)

        ACAN = TLIBCANFD(0,8,0X1,1,0,[1,2,3,4,5,6,7,8])

        tsapp_transmit_canfd_async(hwHandle,ACAN)

        time.sleep(2)

        TCANBuffer = (TLIBCANFD*100)()
        BufferSize = s32(100)  #buffersize 就是TLINBuffer的长度

        tsfifo_receive_canfd_msgs(hwHandle,TCANBuffer,BufferSize,0,READ_TX_RX_DEF.ONLY_RX_MESSAGES) 



        # tsapp_unregister_event_canfd_whandle(hwHandle,ONCAN)



        tsapp_disconnect_by_handle(hwHandle)

        finalize_lib_tscan()
