/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-08-01 01:07:00
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-15 21:49:17
 * @FilePath: /window_linux_Rep/c++/TSFlexray/src/ini_read.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include "iniHeader.h"
#include "TSCANDef.hpp"
#include<cstring>
// #include<algorIThm>
#include"TSBUSConfig.cpp"
#include"TSCAN.hpp"
// #include"windows.h"
#include"TSSocket.h"
#include<cstdio>
#include<bitset>
#include"stdc++.h"
#include <math.h>
#include <unistd.h>


typedef struct _TE2EFrame
{
	u16 FID;
	u8 BaseCycle;
	u8 RepCycle;
	u16 FDataId;
	u8 FData[64];
	std::vector<signal_parse> Signals;
}TE2EFrame,*PE2EFrame;


#define MAX_Frame_LEN 124
#define FConfig "config"
#define FFrameSlotID "FFrameID"
#define FBaseCycle "FBaseCycle"
#define FRepCycle "FRepCycle"
#define FFrameLen "FFrameLen"
#define FDataID "FDataId"
#define E2EConfig "E2EConfig"
#define SaveLog "SaveLog"
#define FDeviceSerial "FDeviceSerial"

#define CoolCode  0x31
#define NormalCode  0x1
#define OneSend  0xA9

ini::iniReader config;

bool is_stop = false;

#if defined(_WIN32)
SOCKET SocketServer;

SOCKET SocketClient;

sockaddr_in addr;
#endif
#if defined(__linux__)
#include "ASCWriteDef.hpp"
int SocketServer;

int SocketClient;

sockaddr_in addr;
#endif

bool SocketIsConnect = false;

u64 Handle = 0;

u64 Handlecan = 0;

int FrameLens[MAX_Frame_LEN] = { 0 };

int FrameLens_1[MAX_Frame_LEN] = { 0 };

int len = 124;

int len_1 = 124;

TLibTrigger_def Frames[MAX_Frame_LEN] = {0};

TLibTrigger_def Frames_1[MAX_Frame_LEN]= {0};

std::vector<TE2EFrame> E2EFrames;
std::vector<TE2EFrame> E2EFrames_1;

int E2EEnable = 0;

int EnablaLogging = 0;

std::string ADeviceSerial;

std::string CRCDll;

u8 crc8_calc(u8* data,s32 len,u16 dataId)
{
	int i, j, k;
	u8 Rtn;
	u8 data2[32];
	s32 len1;
	Rtn = 00^0X00;
	len1 = sizeof(data)/sizeof(data[0]); 
	for(k = 0; k < len + 2; k++) {
	if(k == 0) {
		data2[k] = dataId & 0x00FF;
	} else if(k == 1) {
		data2[k] = (dataId & 0xFF00) >> 8;
	} else {
		data2[k] = data[k-2];
	}
	}
	for (i = 0; i < len + 2; i++) {
	Rtn = Rtn ^ data2[i];
	for (j = 0; j < 8; j++) {
		if (Rtn & 0x80) {
		Rtn = (Rtn << 1) ^ 0x1D;
		} else {
		Rtn = Rtn << 1;
		}
	}
	}
	return Rtn;
}

bool a_less_b(const TLibTrigger_def& r,const TLibTrigger_def& s)
{
    return r.slot_id < s.slot_id;
}

bool Open_ini(const char* fileName)
{
	return config.ReadConfig(fileName);
}

void Stringsplit(const std::string& str, const std::string& splits, std::vector<std::string>& res)
{
	if (str == "")		return;
	//在字符串末尾也加入分隔符，方便截取最后一段
	std::string strs = str + splits;
	size_t pos = strs.find(splits);
	int step = splits.size();
	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos)
	{
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + step, strs.size());
		pos = strs.find(splits);
	}
}

void ReadConfig()
{
	E2EEnable = config.ReadInt(FConfig,E2EConfig,1);
	EnablaLogging = config.ReadInt(FConfig,SaveLog,1);
	ADeviceSerial = config.ReadString(FConfig,FDeviceSerial,"");
	CRCDll = config.ReadString(FConfig,FDeviceSerial,"");
}

void ReadFrames(ini::iniReader config,int Chnidx,PLibTrigger_def Triggers,int* Framelens,int* len,std::vector<TE2EFrame>&E2EFrames)
{
	TLibTrigger_def Trigger;
	std::vector<TLibTrigger_def> Frames;
	TE2EFrame Frame;
	signal_parse SignalDef;
	memset(&Frame,0,sizeof(TE2EFrame));
	memset(&SignalDef,0,sizeof(signal_parse));
	memset(&Trigger,0,sizeof(TLibTrigger_def));

	int FrameIDX = 0 ;
	for(int i =0;i<MAX_Frame_LEN;i++)
	{
		memset(&Frame,0,sizeof(TE2EFrame));
		if (config.section_exists((std::to_string(Chnidx)+std::to_string(i)).c_str()))
		{
			int ret = config.ReadInt((std::to_string(Chnidx)+std::to_string(i)).c_str(),FFrameSlotID,0);
			if(ret == 0)
			{	
				return ;
			}
			Trigger.slot_id = (u16)ret;
			Frame.FID = (u16)ret;
			Frame.BaseCycle = (u8)(config.ReadInt((std::to_string(Chnidx)+std::to_string(i)).c_str(),FBaseCycle,0));
			Frame.RepCycle = (u8)(config.ReadInt((std::to_string(Chnidx)+std::to_string(i)).c_str(),FRepCycle,1));
			Trigger.cycle_code = 1 ;//(u8)(Frame.BaseCycle + Frame.RepCycle);
			Trigger.frame_idx = FrameIDX;
			Trigger.config_byte = 1;
			//config.ReadInt((std::to_string(Chnidx)+std::to_string(i)).c_str(),FFrameLen,32);
			Frames.push_back(Trigger);
			FrameIDX++;
			int Framelen = config.ReadInt((std::to_string(Chnidx)+std::to_string(i)).c_str(),FFrameLen,32);
			Framelens[i] = Framelen;
			
			for(int E2EIndex = 0;E2EIndex<MAX_Frame_LEN;E2EIndex++)
			{
				
				if(config.key_exists((std::to_string(Chnidx)+std::to_string(i)).c_str(),std::to_string(E2EIndex).c_str()))
				{
					std::string str = config.ReadString((std::to_string(Chnidx)+std::to_string(i)).c_str(),std::to_string(E2EIndex).c_str(),"1,0,1");
					std::vector<std::string>res;
					Stringsplit(str,",",res);
					SignalDef.start_bit = std::stoi(res[0]);
					SignalDef.bitlen = (u8)(std::stoi(res[1]));
					SignalDef.is_intel = (u8)(std::stoi(res[2]));
					SignalDef.is_unSigned = 1;
					SignalDef.factor = 1;
					SignalDef.offset = 0;
					Frame.FDataId = (u16)config.ReadInt((std::to_string(Chnidx)+std::to_string(i)).c_str(),FDataID,0);;
					Frame.Signals.push_back(SignalDef);
				}
				else if(Frame.FDataId!=0)
				{
					E2EFrames.push_back(Frame);
					break;
				}
				else 
				{
					break;
				}
				

			}
			
		}
		else
		{
			if(Frames.size() == 0)
				return ;
			sort(Frames.begin(),Frames.end(),a_less_b);
			*len = Frames.size();
			for(int j = 0;j<Frames.size();j++)
			{
				
				if(Frames[j].frame_idx == Frames[0].frame_idx)
				{
					Frames[j].config_byte = CoolCode;
				}
				else if(Frames[j].frame_idx<61)
				{
					Frames[j].config_byte = NormalCode;
				}
				else
				{
					Frames[j].config_byte = OneSend;
				}
				Triggers[j] = Frames[j];
			}
			return ;
		}

	}
	return ;
} 
#if defined(_WIN32)
void __stdcall OnFlexray(size_t* obj,const PLibFlexRay AData)
#endif
#if defined(__linux__)
void OnFlexray(size_t* obj,const PLibFlexRay AData)
#endif
{
	if(AData->FCCType == 0)
	{
		if(AData->FIdxChn == 0)
		{
			if(AData->FDir == 1)
			{
				AData->FChannelMask |= 4;
				for(int i =0;i<E2EFrames.size();i++)
				{
					if(AData->FSlotId == E2EFrames[i].FID&&(AData->FCycleNumber%E2EFrames[i].RepCycle == E2EFrames[i].BaseCycle))
					{
						E2EFrames[i].FData[0]++;
						if(E2EFrames[i].FData[0]>14)
							E2EFrames[i].FData[0] = 0;
						//printf("value = %f\r\n",get_signal_value(AData->FData,&E2EFrames[i].Signals[0]));
						tsflexray_transmit_async(Handle,AData);
						break;
					}
				}
			}//SocketIsConnect = false;
			if(SocketIsConnect){
			char* Buffer = (char*)AData;
			send_socket(SocketClient,Buffer,sizeof(TLibFlexRay));
			}
		}
		else if (AData->FIdxChn == 1)
		{
			if(AData->FDir == 1)
			{
				AData->FChannelMask |= 4;
				for(int i =0;i<E2EFrames_1.size();i++)
				{
					if(AData->FSlotId == E2EFrames_1[i].FID&&(AData->FCycleNumber%E2EFrames_1[i].RepCycle == E2EFrames_1[i].BaseCycle))
					{
						E2EFrames_1[i].FData[0]++;
						if(E2EFrames_1[i].FData[0]>14)
							E2EFrames_1[i].FData[0] = 0;
						tsflexray_transmit_async(Handle,AData);
						break;
					}
				}
			}//SocketIsConnect = false;
			if(SocketIsConnect){
				char* Buffer = (char*)AData;
				send_socket(SocketClient,Buffer,sizeof(TLibFlexRay));
				}
		}
	}
}
#if defined(_WIN32)
void __stdcall PreTxFlexray(size_t* obj,const PLibFlexRay AData)
#endif
#if defined(__linux__)
void PreTxFlexray(size_t* obj,const PLibFlexRay AData)
#endif
{
	if(AData->FCCType == 0)
	{
		
		if(AData->FIdxChn == 0)
		{
			for(int i =0;i<E2EFrames.size();i++)
				{
					if(AData->FSlotId == E2EFrames[i].FID&&(AData->FCycleNumber%E2EFrames[i].RepCycle == E2EFrames[i].BaseCycle))
					{
						s32 realLen = 0;
						s32 datalen = E2EFrames[i].Signals.size();
						set_signal_value(&E2EFrames[i].Signals[0],AData->FData,E2EFrames[i].FData[0]);
						set_signal_value(&E2EFrames[i].Signals[datalen-1],AData->FData,1);
						realLen++;
						for(s32 sgnidx = 1;sgnidx < datalen-2;sgnidx++)
						{
							s32 SignalLen = E2EFrames[i].Signals[sgnidx].bitlen / 9 + 1;
							s32 value = get_signal_value(&E2EFrames[i].Signals[sgnidx],AData->FData);
							for(s32 slen =0 ;slen < SignalLen;slen++)
							{
								E2EFrames[i].FData[realLen] = (value>>(8*slen))&0xff;
								realLen++;
							}
						}
						if(E2EEnable)
							set_signal_value(&E2EFrames[i].Signals[datalen-2],AData->FData,crc8_calc(E2EFrames[i].FData,realLen,E2EFrames[i].FDataId));
						return;
					}
				}
		}
		else if (AData->FIdxChn == 1)
		{
			if(AData->FDir == 1)
			{
				for(int i =0;i<E2EFrames_1.size();i++)
				{
					if(AData->FSlotId == E2EFrames_1[i].FID&&(AData->FCycleNumber%E2EFrames_1[i].RepCycle == E2EFrames_1[i].BaseCycle))
					{
						s32 realLen = 0;
						s32 datalen = E2EFrames_1[i].Signals.size() ;
						set_signal_value(&E2EFrames_1[i].Signals[0],AData->FData,E2EFrames_1[i].FData[0]);
						set_signal_value(&E2EFrames_1[i].Signals[datalen-1],AData->FData,1);
						realLen++;
						for(s32 sgnidx = 1;sgnidx < datalen-2;sgnidx++)
						{
							s32 SignalLen = E2EFrames_1[i].Signals[sgnidx].bitlen / 9 + 1;
							s32 value = get_signal_value(&E2EFrames_1[i].Signals[sgnidx],AData->FData);
							for(s32 slen =0 ;slen < SignalLen;slen++)
							{
								E2EFrames_1[i].FData[realLen] = (value>>(8*slen))&0xff;
								realLen++;
							}
						}
						if(E2EEnable)
							set_signal_value(&E2EFrames_1[i].Signals[datalen-2],AData->FData,crc8_calc(E2EFrames_1[i].FData,realLen,E2EFrames_1[i].FDataId));
						break;
					}
				}
			}
		}
		
	}
}

bool Create_SocketServer(const char* ip,const int port,int backlog)
{
	#if defined(__linux__)
	int opt = 1; 
	if(!init_tcp_socket(SocketServer,opt))
		return false;
	if(!bind_socket(SocketServer,ip,port))
		return false;
	if(!listen_socket(SocketServer,backlog))
		return false;
	return true;
	#endif
	#if defined(_WIN32)
	if(!init_socket())
		return false;
	if(!create_socket(&SocketServer))
		return false;
	if(!bind_socket(SocketServer,ip,port))
		return false;
	if(!listen_socket(SocketServer,backlog))
		return false;
	return true;
	#endif
}

bool accept_Socket()
{
	#if defined(__linux__)
	if(!accept_socket(SocketServer,addr,SocketClient))
		return false;
	SocketIsConnect = true;
	return true;
	#endif
	#if defined(_WIN32)
	if(!accept_socket(SocketServer,&SocketClient,&addr))
		return false;
	SocketIsConnect = true;
	printf("accept success ip = %s, port = %d,\r\n",inet_ntoa(addr.sin_addr),addr.sin_port);
	return true;
	#endif
}

std::string get_time()
{
    //获取系统时间戳
	char NowTime[50] ={0}; 
	time_t timeReal;
	time(&timeReal);
	timeReal = timeReal + 8*3600;
	tm* t = gmtime(&timeReal); 
	sprintf(NowTime,"%d-%02d-%02d_%02d_%02d_%02d.blf", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec); 
	return NowTime;
}

int main() 
{
	initialize_lib_tscan(true,true,false);
	if(!Open_ini("config.ini"))
		return 0xff;
	ReadConfig();
	ReadFrames(config,0,Frames,FrameLens,&len,E2EFrames);
	ReadFrames(config,1,Frames_1,FrameLens_1,&len_1,E2EFrames_1);
	int tsret = tscan_connect(ADeviceSerial.c_str(),&Handle);
	tsflexray_register_event_flexray_whandle(Handle,OnFlexray);
	tsflexray_register_pretx_event_flexray_whandle(Handle,PreTxFlexray);
	#if defined(__linux__)
		if(EnablaLogging)
		    blf_start_logging(Handle,get_time().c_str());
	#endif

	tsret = tsflexray_stop_net(Handle,0,1000);
	tsret = tsflexray_stop_net(Handle,1,1000);
	printf("ret = %d\r\n",tsret);
	if(tsret !=0 &&tsret != 5)
		return -1;
	if( 0 != InitFlexrayNode(Handle,0,Frames,FrameLens,len))
	{
		finalize_lib_tscan();
		return -2;
	}
	if( 0 != InitFlexrayNode(Handle,1,Frames_1,FrameLens_1,len_1))
	{
		finalize_lib_tscan();
		return -2;
	}
	//Sleep(3000);
	tsret = tsflexray_start_net(Handle,0,1000);
	tsret = tsflexray_start_net(Handle,1,1000);

	if( 0 == set_can_config("609C84C2E4BAF720",12,&Handlecan))
		{
			for( int i = 0;i<12;i++)
				set_cycle_msg(Handlecan,i);
		}

	if(Create_SocketServer("127.0.0.1",8000,1))
		accept_Socket();
	while(!is_stop)
	{
		sleep(1);
	}
	tscan_disconnect_all_devices();
	finalize_lib_tscan();
	return 0;
}