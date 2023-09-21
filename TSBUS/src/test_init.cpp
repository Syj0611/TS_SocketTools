#include <string>
#include "iniHeader.h"
#include<cstring>
#include"TSCANDef.hpp"
#include"TSBUSConfig.cpp"
#include<iostream>
#include"TSCAN.hpp"
#include"TSMSGStruct.h"


int main(){
    initialize_lib_tscan(true,false,false);

    tscan_config_canfd_by_baudrate(0,(APP_CHANNEL)CHN1,500,2000,lfdtISOCAN,lfdmNormal,1);

    TLibCANFD AMsg;
    AMsg.FIdentifier = 0x127;
    AMsg.FDLC = 8;
    u8 isfd = 0;
    u8 isbrs = 0;
    u8 isstd = 0;
    u16 Cycletime = 0.01;
    AMsg.SetIsFD((isfd==1));
    AMsg.SetStd(isstd==1);
    AMsg.SetIsBRS(isbrs ==1);
    tscan_add_cyclic_msg_canfd(0,&AMsg,Cycletime*1000);

    tscan_config_canfd_by_baudrate(0,(APP_CHANNEL)CHN1,500,2000,lfdtISOCAN,lfdmNormal,1);
    AMsg.FIdentifier = 0x401;
    AMsg.FDLC = 8;
    isfd = 0;
    isbrs = 0;
    isstd = 0;
    Cycletime = 0.01;
    AMsg.SetIsFD((isfd==1));
    AMsg.SetStd(isstd==1);
    AMsg.SetIsBRS(isbrs ==1);
    tscan_add_cyclic_msg_canfd(0,&AMsg,Cycletime*1000);

    
}
