/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-08-25 00:44:42
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-25 05:02:51
 * @FilePath: /TSBUS_SocketTools/TSCANDemo/src/test.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "TSCANDef.hpp"
#include "iniHeader.h"
#include <vector>
#include <cstring>
#include"ASCWriteDef.hpp"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
using namespace std;

ini::iniReader config;

void readini(ini::iniReader config,vector<TLibCANFD>&AMsgs)
{
    TLibCANFD msg;
    memset(&msg,0,sizeof(msg));
    for(int chnidx =0 ;chnidx<12;chnidx++)
    {
        for(int i =0;i<200;i++)
        {
            string section = to_string(chnidx)+to_string(i);
            if(!config.section_exists(section.c_str()))
            {
                break;
            }
            msg.FIdentifier = config.ReadInt(section.c_str(),"CANID",0);
            msg.FIdxChn = chnidx;
            msg.FDLC = 8;
            AMsgs.push_back(msg);
        }
    }
}

string get_time(int hwindex)
{
    //获取系统时间戳
	char NowTime[50] ={0}; 
	time_t timeReal;
	time(&timeReal);
	timeReal = timeReal + 8*3600;
	tm* t = gmtime(&timeReal); 
	sprintf(NowTime,"%d-%02d-%02d_%02d_%02d_%02d_%d.asc", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,hwindex); 
	return NowTime;
}

vector<TLibCANFD>AMsgs;
u64 Handle = 0 ;
FILE* file;
int main()
{
    initialize_lib_tscan(true,false,false);

    u32 ACount = 0;

    tscan_scan_devices(&ACount);

    cout<<"find "<<ACount<<" devices"<<endl;
    
    cout<<"start print devices info "<<endl;


    for(s32 i =0;i<ACount;i++)
    {
        char* AFManufacturer ;
        char* AFProduct ;
        char* AFSerial ;
        tscan_get_device_info(i,&AFManufacturer,&AFProduct,&AFSerial);
        cout<<"device "<<i<<" info:"<<AFManufacturer<<" "<<AFProduct<<" "<<AFSerial<<endl;
    }

    // s32 ret =  tscan_connect("",&Handle);
    // for(int canCountidx = 0;canCountidx<2;canCountidx++ )
    // {
    // tscan_config_canfd_by_baudrate(Handle,(APP_CHANNEL)canCountidx,500,2000,lfdtISOCAN,lfdmNormal,1);
    // }
    // tslog_write_start(get_time(0).c_str(),&file);
    // config.ReadConfig("config1.ini");
    // readini(config,AMsgs);
    // for(int i =0;i<AMsgs.size();i++)
    // {
    //     tscan_add_cyclic_msg_canfd(Handle,&AMsgs[i],20);
    // }
    // while (1)
    // {
    //     TLibCANFD ACANFDMsg[1];
    //     s32 ACANFDMsgSize =1;
    //     s32 ret = tsfifo_receive_canfd_msgs(Handle,ACANFDMsg,&ACANFDMsgSize,0xff,1);
    //     if(ACANFDMsgSize!=0 )
    //     {

    //             tslog_write_can(file,&ACANFDMsg[0]);
    //     }
    //     if(ACANFDMsgSize==0)
    //         usleep(1);
    // }
    
}