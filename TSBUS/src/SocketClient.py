'''
Author: seven 865762826@qq.com
Date: 2023-08-04 22:31:39
LastEditors: seven 865762826@qq.com
LastEditTime: 2023-08-21 21:21:31
FilePath: \window_linux_Rep\c++\TSFlexray\src\SocketClient.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
from libTSCANAPI import *

import socket
import struct
# import vthread

class TLIBFlexrayHW(Structure):
    _pack_ = 1
    _fields_ = [("FHWIdx", c_int32),
                ("AMsg", TLIBFlexray),
    ]
    def __str__(self):
        return str(self.FHWIdx) +"  "+str(self.AMsg)
PLIBFlexrayHW = POINTER(TLIBFlexrayHW)   

class TLIBCANFDHW(Structure):
    _pack_ = 1
    _fields_ = [("FHWIdx", c_int32),
                ("CyclicTime",c_float),
                ("AMsg", TLIBCANFD),
    ]
    def __str__(self):
        return str(self.FHWIdx) +"  "+str(self.AMsg)
PLIBCANFDHW = POINTER(TLIBCANFDHW) 

# create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# set the host and port number
host = '127.0.0.1'
port = 8003

# connection to host on the port.
client_socket.bind((host, port))

client_socket.sendto("nihao".encode("utf8"), (host, 8000))
a = True
while True:
    # time.sleep(0.01)
    msg, addr = client_socket.recvfrom(1024)
    if(len(msg)== 308):
        Msg = cast(msg, PLIBFlexrayHW).contents
        # if(Msg.AMsg.FSlotId==55 and Msg.AMsg.FCycleNumber % 1 ==0):
        #     Msg.AMsg.FData[11] &= 0xF0
        #     Msg.AMsg.FData[11] |= 3
        #     if a:
        #         a = False
        #         client_socket.sendto(Msg,(host, 8000))
        print(Msg)
        pass
    elif(len(msg) == 88):
        Msg = cast(msg, PLIBCANFDHW).contents
        
        if a:
            AMsg = TLIBCANFD(7,8,0x100,1,1,[1,2,3,4,5,6,7,8])
            Msg.AMsg = AMsg
            Msg.CyclicTime = 0.1
            a = False
            client_socket.sendto(Msg,(host, 8002))
        # print(Msg)
    # Msg.FData[0] = 0xf1
    

    # client_socket.sendto(Msg,(host, 8000))

        # frlist.append(str(Msg))


client_socket.close()



