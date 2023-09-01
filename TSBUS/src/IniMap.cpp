/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-08-09 15:17:28
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-25 00:34:56
 * @FilePath: \window_linux_Rep\c++\TSFlexray\src\IniMap.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <string>
#include <vector>
#include <map>
#include<queue>
#include<fstream>
#include "iniHeader.h"
#include<cstring>
#include"TSCANDef.hpp"
#include"TSBUSConfig.cpp"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include "TSSocket.h"
#include <stdexcept>
#include<iostream>
#include<pthread.h>
#include"TSCAN.hpp"
#include"TSMSGStruct.h"
#include"ASCWriteDef.hpp"



#define MaxCount  12
#define MaxMsgCount  124
#define MaxSiganlCount  64
#define OneE2EMaxCount 8
#define BUSTYPES 3   //0:flexray 1:can 2:lin

#define FConfig "config"
#define FFrameSlotID "FFrameID"
#define FBaseCycle "FBaseCycle"
#define FRepCycle "FRepCycle"
#define FFrameLen "FFrameLen"
#define FDataID "FDataId"
#define E2EConfig "E2EConfig"
#define SaveLog "SaveLog"
#define FDeviceSerial "DeviceSerial"

#define FlexraySRCIP "FlexRaySRCIP"
#define FlexRaySRCPORT "FlexRaySRCPORT"
#define FlexRayDSTIP "FlexRayDSTIP"
#define FlexRayDSTPORT "FlexRayDSTPORT"

#define CANMSGID "CANID"
#define ISCANFD "ISCANFD"
#define CANFDBRS "CANFDBRS"
#define ISSTD "ISSTD"
#define CANCyclcTime "CyclcTime"
#define CANMSGLEN "CANLEN"
//0-flexray 1-can 2-lin
typedef struct _SaveMsg{
    int HWIdx;
    int MsgType;
    char AMsg[400];
} TSaveMsg,*PSaveMsg;

string get_time(const char* hwindex);

vector<u64> HandleList;

vector<FILE*> blf_BLHANDLEList;

queue<TSaveMsg>HWMsgs;

//vector HW< vector< BUSType< channeList> > > 
//
//vector<map<int,chnlist>>
//  hw = vector<chns>
//  chns  chnidx : businfo
//  businfo  bustype  :  ChnList
vector<map<uint32_t,map<uint32_t,vector<map<uint64_t, vector<vector<signal_parse>>>>>>> MappingTable;

// map<uint32_t,map<uint64_t, vector<vector<signal_parse>>>> ChnList;

ini::iniReader config;

bool SocketIsConnect = false;

u8 ISSaveBLF = 0;

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

sockaddr_in dstaddr;

#endif

string flexraysrcip;
int flexraysrcport = 0;
string flexraydstip;
int flexraydstport = 0;
u8 E2ECale = 1;
vector<string>ADeviceSerials;

// u8 crc8_calc(u8* data,s32 len,u16 dataId)
// {
// 	int i, j, k;
// 	u8 Rtn;
// 	u8 data2[32];
// 	s32 len1;
// 	Rtn = 00^0X00;
// 	len1 = sizeof(data)/sizeof(data[0]); 
// 	for(k = 0; k < len + 2; k++) {
// 	if(k == 0) {
// 		data2[k] = dataId & 0x00FF;
// 	} else if(k == 1) {
// 		data2[k] = (dataId & 0xFF00) >> 8;
// 	} else {
// 		data2[k] = data[k-2];
// 	}
// 	}
//     for (i = 0; i < len + 2; i++) {
// 	Rtn = Rtn ^ data2[i];
// 	for (j = 0; j < 8; j++) {
// 		if (Rtn & 0x80) {
// 		Rtn = (Rtn << 1) ^ 0x11D;
// 		} else {
// 		Rtn = Rtn << 1;
// 		}
// 	}
// 	}
// 	return Rtn;
// }


u8 crc8_calc(u8* data,s32 len,u16 dataId)
{
    u8 Start_Value = 0;
    u8 XOR_Value = 0;
    u8 t_crc = Start_Value;
    int i = 0;
    while(i<len)
    {
        t_crc ^= data[i];
        int b = 0;
        while (b<8)
        {
            if((t_crc &0x80)!=0)
            {
                t_crc<<=1;
                t_crc^=dataId;
            }
            else
                t_crc<<=1;
            b++;
        }
        i++;
    }
    t_crc ^= XOR_Value;
    return t_crc;
}

void Stringsplit(const string& str, const string& splits, vector<string>& res)
{
	if (str == "")		return;
	//在字符串末尾也加入分隔符，方便截取最后一段
	string strs = str + splits;
	size_t pos = strs.find(splits);
	int step = splits.size();
	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + step, strs.size());
		pos = strs.find(splits);
	}
}
bool Open_ini(const char* fileName)
{
	return config.ReadConfig(fileName);
}

bool Read_ini_Config(ini::iniReader config)
{
    string DeviceSerials = config.ReadString(FConfig,FDeviceSerial,"");
    ISSaveBLF = (u8)config.ReadInt(FConfig,SaveLog,0);
    Stringsplit(DeviceSerials,",",ADeviceSerials);
    for(int i =0;i<ADeviceSerials.size();i++)
    {
        u64 HWHandle;
        s32 ret = tscan_connect(ADeviceSerials[i].c_str(),&HWHandle);
        if(0 == ret)
        {
            HandleList.push_back(HWHandle);
            if(ISSaveBLF)
            {
                FILE* blfhandle ;
                tslog_write_start(get_time(ADeviceSerials[i].c_str()).c_str(),&blfhandle);
                blf_BLHANDLEList.push_back(blfhandle);
            }
        }
        else
        {
            return false;
        }
    }
    flexraysrcip = config.ReadString(FConfig,FlexraySRCIP,"127.0.0.1");
    flexraysrcport = config.ReadInt(FConfig,FlexRaySRCPORT,8000);
    flexraydstip = config.ReadString(FConfig,FlexRayDSTIP,"127.0.0.1");
    flexraydstport = config.ReadInt(FConfig,FlexRayDSTPORT,8001);
    E2ECale = (u8)config.ReadInt(FConfig,E2EConfig,8001);
    return true;
}


//vector<map<int,chnlist>>
//  hw = vector<chns>
//  chns  chnidx : businfo
//  businfo  bustype  :  ChnList
void Read_ini(ini::iniReader config,vector<map<uint32_t,map<uint32_t,vector<map<uint64_t, vector<vector<signal_parse>>>>>>>&MappingTable)//u8** Framelen,TLibTrigger_def** Trigger_def)
{
    map<uint32_t,map<uint32_t,vector<map<uint64_t, vector<vector<signal_parse>>>>>> CHNListInfo;
    map<uint32_t,vector<map<uint64_t, vector<vector<signal_parse>>>>> BUSInfo; 
    vector<map<uint64_t, vector<vector<signal_parse>>>> ChnList;
    vector<signal_parse> SignlList;
    map<uint64_t,vector<vector<signal_parse>>> MsgList;
    vector<vector<signal_parse>> E2ECountSignals;
    uint64_t FlexrayMsg ;
    signal_parse SignalMsg;
    string strsection;
    TLibFlexRay FRMsg;
    memset(&FRMsg,0,sizeof(FRMsg));
    memset(&FlexrayMsg,0,sizeof(FlexrayMsg));
    memset(&SignalMsg,0,sizeof(signal_parse));
    for(int hwidx =0;hwidx<ADeviceSerials.size();hwidx++)
    {
    //循环通道：  //hw chn bus 
        for(int i=0;i<MaxCount;i++)
        {
            // fleray报文 最大数量 循环
            for(int busidx =0;busidx <BUSTYPES;busidx++)
            {
                switch (busidx)
                {
                case 0:
                    for(int Msgidx = 0;Msgidx<MaxMsgCount;Msgidx++)
                    {
                        strsection =to_string(hwidx)+to_string(busidx)+to_string(i)+to_string(Msgidx);
                        if(!config.section_exists(strsection.c_str()))
                        {
                            if(MsgList.size()>0)
                            {
                                ChnList.push_back(MsgList);
                                MsgList.clear();
                                
                            }
                            //break;
                        }/* code */
                        else{
                        u16 Fmsgslotid = (u16)config.ReadInt(strsection.c_str(),FFrameSlotID,0);
                        u8 FmsgBasecyc = (u8)config.ReadInt(strsection.c_str(),FBaseCycle,0);
                        u8 Fmsgrepcyc = (u8)config.ReadInt(strsection.c_str(),FRepCycle,1);
                        u8 Fmsglen = (u8)config.ReadInt(strsection.c_str(),FFrameLen,32);
                        FlexrayMsg = (((uint64_t)Fmsgslotid)<<48)+(((uint64_t)FmsgBasecyc)<<40)+((uint64_t)Fmsgrepcyc<<32)+(((uint64_t)Fmsglen));
                        E2ECountSignals.clear();
                        // calc one msg has e2e count
                        for (int E2Eidx =0;E2Eidx<OneE2EMaxCount;E2Eidx++)
                        {
                        SignlList.clear();
                        string e2ekey = to_string(E2Eidx) + "0";
                        if(config.key_exists(strsection.c_str(),e2ekey.c_str()))
                        {
                        for(int SignalIdx = 0;SignalIdx<MaxSiganlCount;SignalIdx++)
                        {
                            e2ekey = to_string(E2Eidx) + to_string(SignalIdx);
                            if(config.key_exists(strsection.c_str(),e2ekey.c_str()))
                            {
                                string str = config.ReadString(strsection.c_str(),e2ekey.c_str(),"0,0,0,0,0,0");
                                vector<string> strs;
                                Stringsplit(str,",",strs);
                                int start_bit = stoi(strs[0]);
                                SignalMsg.bitlen = stoi(strs[1]);
                                SignalMsg.is_intel = stoi(strs[2]);
                                SignalMsg.start_bit = start_bit;//calcrealstartbit(start_bit,SignalMsg.bitlen,SignalMsg.is_intel);
                                SignalMsg.dataid = (u16)stoi(strs[3]);
                                SignalMsg.is_unSigned = 1;
                                SignalMsg.offset = 0;
                                SignalMsg.factor = 1;
                                SignalMsg.value = (double)strtof(strs[4].c_str(),NULL);
                                SignalMsg.is_cntr = (u8)strtof(strs[5].c_str(),NULL);
                                SignalMsg.AMsg.FIdxChn = (u8)i;
                                SignalMsg.AMsg.FActualPayloadLength = Fmsglen;
                                SignalMsg.AMsg.FChannelMask = 5;
                                SignalMsg.AMsg.FCycleNumber = FmsgBasecyc+Fmsgrepcyc;
                                SignalMsg.AMsg.FPayloadLength = 254;
                                SignalMsg.AMsg.FSlotId = Fmsgslotid;
                                get_real_signal(&SignalMsg);
                                SignlList.push_back(SignalMsg);
                            }
                            else{
                                if(SignlList.size()>0)
                                {
                                    E2ECountSignals.push_back(SignlList);
                                    SignlList.clear();
                                }
                                break;
                            }
                        }
                        }
                        else
                        {
                            if(E2ECountSignals.size()>0)
                            {
                                MsgList[FlexrayMsg] = E2ECountSignals;
                                E2ECountSignals.clear();
                            }
                            else
                            {
                                MsgList[FlexrayMsg] = E2ECountSignals;
                                E2ECountSignals.clear();
                            }
                            break;
                        }
                        }
                        }
                    }            
                    break;
                case 1:
                // 
                    for(int Msgidx = 0;Msgidx<200;Msgidx++)
                    {
                        char chn[2] = {0};
                        sprintf(chn,"%X",i);
                        string chnstr = chn;
                        strsection =to_string(hwidx)+to_string(busidx)+chnstr+to_string(Msgidx);
                        if(!config.section_exists(strsection.c_str()))
                        {
                            if(MsgList.size()>0)
                            {
                                ChnList.push_back(MsgList);
                                MsgList.clear();
                                
                            }
                            //break;
                        }/* code */
                        else{
                            // CANID = 1
                            // ISCANFD = 1
                            // CANFDBRS = 1
                            // ISExt = 1
                            // CyclcTime = 100
                            // CANLEN = 8
                        s32 canid = config.ReadInt(strsection.c_str(),CANMSGID,0);
                        u8 canlen = (u8)config.ReadInt(strsection.c_str(),CANMSGLEN,8);
                        u16 cycletime = (u16)((float)config.ReadFloat(strsection.c_str(),CANCyclcTime,0) * 1000);
                        u8 isfd = (u8)config.ReadInt(strsection.c_str(),ISCANFD,0);
                        u8 isbrs = (u8)config.ReadInt(strsection.c_str(),CANFDBRS,0);
                        u8 isstd = (u8)config.ReadInt(strsection.c_str(),ISSTD,0);

                        FlexrayMsg = (((uint64_t)canid)<<32)+(((uint64_t)canlen)<<24)+((uint64_t)cycletime<<8)+((isfd<<2))+((isbrs<<1))+((isstd));
                        E2ECountSignals.clear();
                        // calc one msg has e2e count
                        for (int E2Eidx =0;E2Eidx<OneE2EMaxCount;E2Eidx++)
                        {
                        SignlList.clear();
                        string e2ekey = to_string(E2Eidx) + "0";
                        if(config.key_exists(strsection.c_str(),e2ekey.c_str()))
                        {
                        for(int SignalIdx = 0;SignalIdx<MaxSiganlCount;SignalIdx++)
                        {
                            e2ekey = to_string(E2Eidx) + to_string(SignalIdx);
                            if(config.key_exists(strsection.c_str(),e2ekey.c_str()))
                            {
                                string str = config.ReadString(strsection.c_str(),e2ekey.c_str(),"0,0,0,0,0,0");
                                vector<string> strs;
                                Stringsplit(str,",",strs);
                                int start_bit = stoi(strs[0]);
                                SignalMsg.bitlen = stoi(strs[1]);
                                SignalMsg.is_intel = stoi(strs[2]);
                                SignalMsg.start_bit = start_bit;//calcrealstartbit(start_bit,SignalMsg.bitlen,SignalMsg.is_intel);
                                SignalMsg.dataid = (u16)stoi(strs[3]);
                                SignalMsg.is_unSigned = 1;
                                SignalMsg.offset = 0;
                                SignalMsg.factor = 1;
                                SignalMsg.value = (double)strtof(strs[4].c_str(),NULL);
                                SignalMsg.is_cntr = (u8)strtof(strs[5].c_str(),NULL);
                                get_real_signal(&SignalMsg);
                                SignlList.push_back(SignalMsg);
                            }
                            else{
                                if(SignlList.size()>0)
                                {
                                    E2ECountSignals.push_back(SignlList);
                                    SignlList.clear();
                                }
                                break;
                            }
                        }
                        }
                        else
                        {
                            if(E2ECountSignals.size()>0)
                            {
                                MsgList[FlexrayMsg] = E2ECountSignals;
                                E2ECountSignals.clear();
                            }
                            else
                            {
                                MsgList[FlexrayMsg] = E2ECountSignals;
                                E2ECountSignals.clear();
                            }
                            break;
                        }
                        }
                        }
                    }            
                    break;
                case 2:
                    break;
                default:
                    break;
                }
            BUSInfo[busidx] = ChnList;
            ChnList.clear();
            }
            CHNListInfo[i] = BUSInfo;
            BUSInfo.clear();
        }
        MappingTable.push_back(CHNListInfo);
        CHNListInfo.clear();
    

    }
}

#if defined(_WIN32)
void __stdcall OnCAN(size_t* obj,const PLibCANFD AData)
#endif
#if defined(__linux__)
void OnCAN(size_t* obj,const PLibCANFD AData)
#endif
{
    TSaveMsg SMsg;
    memset(&SMsg,0,sizeof(SMsg));
    if((AData->FProperties&0X80))
        return;
    int idx ;
    for(idx = 0;idx<HandleList.size();idx++)
    {
        if(HandleList[idx] == (size_t)obj)
            break;
    }
    if(idx>= HandleList.size())
        return;
    SMsg.HWIdx = idx;
    SMsg.MsgType = 1;
    memcpy(SMsg.AMsg,(char*)AData,sizeof(TLibCANFD));
    HWMsgs.push(SMsg);
}

#if defined(_WIN32)
void __stdcall OnPreCAN(size_t* obj,const PLibCANFD AData)
#endif
#if defined(__linux__)
void OnPreCAN(size_t* obj,const PLibCANFD AData)
#endif
{
    if(E2ECale == 1)
    {
    int idx ;
    u8 E2EData[64] = {0};
    for(idx = 0;idx<HandleList.size();idx++)
    {
        if(HandleList[idx] == (size_t)obj)
            break;
    }
    if(idx>= HandleList.size())
        return;
	
    map<uint64_t,vector<vector<signal_parse>>>::iterator it = MappingTable[idx][AData->FIdxChn][1][0].begin();;
    for (;it!=MappingTable[idx][AData->FIdxChn][1][0].end();it++)
    {
        int canid = (u16)((it->first>>32)&0xffffffff);
        bool isstd = (u8)(it->first)&0X1 == 1;
        if((AData->FIdentifier == canid) && (AData->GetStd() == isstd))
        {
            for(int e2eidx =0;e2eidx<it->second.size();e2eidx++)
            {
                s32 realLen = 3;
                s32 datalen = it->second[e2eidx].size();
                E2EData[0] = (u8)(it->second[e2eidx][0].dataid &0xff) ; 
                E2EData[1] = (u8)((it->second[e2eidx][0].dataid >>8)&0xff) ; 
                if(it->second[e2eidx][datalen-1].bitlen !=0)
                {
                    it->second[e2eidx][datalen-1].value = 1;
                    set_signal_value(&it->second[e2eidx][datalen-1],AData->FData);//,(int)DLC_DATA_BYTE_CNT[AData->FDLC]);
                }
                for(int sgnidx =0;sgnidx<datalen - 2;sgnidx++)
                {
                    if(it->second[e2eidx][sgnidx].is_cntr)
                    {
                        
                        set_signal_value(&it->second[e2eidx][sgnidx],AData->FData);//,(int)DLC_DATA_BYTE_CNT[AData->FDLC]);
                        E2EData[2] = (u8)it->second[e2eidx][sgnidx].value;
                        it->second[e2eidx][sgnidx].value++;
                        if(it->second[e2eidx][sgnidx].value>14)
                            it->second[e2eidx][sgnidx].value = 0;
                    }    
                    else{                            
                        s32 SignalLen = it->second[e2eidx][sgnidx].bitlen / 9 + 1;
                        get_signal_value(&it->second[e2eidx][sgnidx],AData->FData);//,(int)DLC_DATA_BYTE_CNT[AData->FDLC]);
                        u64 value = it->second[e2eidx][sgnidx].value;
                        for(s32 slen =0 ;slen < SignalLen;slen++)
                        {
                            E2EData[realLen] = (value>>(8*slen))&0xff;
                            realLen++;
                        }
                    }
                }
                it->second[e2eidx][datalen-2].value = crc8_calc(E2EData,realLen,0X11D);
                set_signal_value(&it->second[e2eidx][datalen-2],AData->FData);//,(int)DLC_DATA_BYTE_CNT[AData->FDLC]);
            }
        }
    }
    }
}
#if defined(_WIN32)
void __stdcall On_tx_rxFlexray(size_t* obj,const PLibFlexRay AData)
#endif
#if defined(__linux__)
void On_tx_rxFlexray(size_t* obj,const PLibFlexRay AData)
#endif
{
    int idx ;
    
    for(idx = 0;idx<HandleList.size();idx++)
    {
        if(HandleList[idx] == (size_t)obj)
            break;
    }
    if(idx>= HandleList.size())
        return;
    if(AData->FSlotId >2048)
        return;
    // TSaveMsg SMsg;
    // SMsg.HWIdx = 0;
    // SMsg.MsgType = 0;
    // memcpy(SMsg.AMsg,(char*)AData,sizeof(TLibFlexRay));
    // HWMsgs.push(SMsg);
    TLibFlexRayHW FRMSG ;
    FRMSG.HWIdx = idx;
    FRMSG.AMsg = *(AData);
    udp_send_socket(SocketServer,(char*)(&FRMSG),sizeof(TLibFlexRayHW),(sockaddr*)&dstaddr);
    if(ISSaveBLF)
    {
        tslog_write_flexray(blf_BLHANDLEList[idx],&FRMSG.AMsg);
    }
}


#if defined(_WIN32)
void __stdcall OnFlexray(size_t* obj,const PLibFlexRay AData)
#endif
#if defined(__linux__)
void OnFlexray(size_t* obj,const PLibFlexRay AData)
#endif
{
    
    if(E2ECale == 1)
    {
    int idx ;
    
    for(idx = 0;idx<HandleList.size();idx++)
    {
        if(HandleList[idx] == (size_t)obj)
            break;
    }
    if(idx>= HandleList.size())
        return;
    if(AData->FSlotId >2048)
        return;
	if(AData->FCCType == 0)
	{

        if(AData->FDir == 1)
        {
            if(AData->FReserved1<5)
            {
        map<uint64_t,vector<vector<signal_parse>>>::iterator it = MappingTable[idx][AData->FIdxChn][0][0].begin();// ChnList[AData->FIdxChn].begin();

        for (;it!=MappingTable[idx][AData->FIdxChn][0][0].end();it++)
        {
            int Fslotid = (u16)((it->first>>48)&0xffff);
            u8 baseCycle = (u8)((it->first>>40)&0xff);
            u8 repCycle = (u8)((it->first>>32)&0xff);
            if(AData->FSlotId == Fslotid && AData->FCycleNumber%repCycle == baseCycle && it->second.size()!=0)
            {
                
                tsflexray_transmit_async((size_t)obj,&it->second[0][0].AMsg);
                // printf("obj = %ld HandleList[0] = %ld  \r\n",obj,HandleList[0]);
            }
        }
        }
        }
    }
    }
}



//vector<map<int,chnlist>>
//  hw = vector<chns>
//  chns  chnidx : businfo
//  businfo  bustype  :  ChnList




#if defined(_WIN32)
void __stdcall PreTxFlexray(size_t* obj,const PLibFlexRay AData)
#endif
#if defined(__linux__)
void PreTxFlexray(size_t* obj,const PLibFlexRay AData)
#endif
{
    if(E2ECale == 1)
    {
    int idx ;
    u8 E2EData[64] = {0};
    for(idx = 0;idx<HandleList.size();idx++)
    {
        if(HandleList[idx] == (size_t)obj)
            break;
    }
    if(idx>= HandleList.size())
        return;
	if(AData->FCCType == 0)
	{
        AData->FChannelMask|=4;
		map<uint64_t,vector<vector<signal_parse>>>::iterator it = MappingTable[idx][AData->FIdxChn][0][0].begin();;
        for (;it!=MappingTable[0][AData->FIdxChn][0][0].end();it++)
        {
            int Fslotid = (u16)((it->first>>48)&0xffff);
            u8 baseCycle = (u8)((it->first>>40)&0xff);
            u8 repCycle = (u8)((it->first>>32)&0xff);
            if(AData->FSlotId == Fslotid && AData->FCycleNumber%repCycle == baseCycle)
            {
                // if(AData->FSlotId == 7 && AData->FCycleNumber%2 ==1)
                // {
                //     printf("11111");
                // }
                for(int e2eidx =0;e2eidx<it->second.size();e2eidx++)
                {
                    
                    s32 realLen = 3;
                    s32 datalen = it->second[e2eidx].size();
                    E2EData[0] = (u8)(it->second[e2eidx][0].dataid &0xff) ; 
                    E2EData[1] = (u8)((it->second[e2eidx][0].dataid >>8)&0xff) ; 
                    if(it->second[e2eidx][it->second[e2eidx].size()-1].bitlen !=0)
                    {
                        it->second[e2eidx][it->second[e2eidx].size()-1].value = 1;
                        set_signal_value(&it->second[e2eidx][it->second[e2eidx].size()-1],AData->FData);//,(int32_t)AData->FActualPayloadLength);
                    }
                    for(int sgnidx =0;sgnidx<datalen - 2;sgnidx++)
                    {
                        if(it->second[e2eidx][sgnidx].is_cntr)
                        {
                            
                            set_signal_value(&it->second[e2eidx][sgnidx],AData->FData);//,(int)AData->FActualPayloadLength);
                            E2EData[2] = (u8)it->second[e2eidx][sgnidx].value;
                            it->second[e2eidx][sgnidx].value++;
                            if(it->second[e2eidx][sgnidx].value>14)
                                it->second[e2eidx][sgnidx].value = 0;
                        }                // if(AData->FSlotId == 7 && AData->FCycleNumber%2 ==1)
                // {
                //     printf("11111");
                // }
                        else{                            
                            s32 SignalLen = it->second[e2eidx][sgnidx].bitlen / 9 + 1;
                            get_signal_value(&it->second[e2eidx][sgnidx],AData->FData);//,(int)AData->FActualPayloadLength);
                            u64 value = it->second[e2eidx][sgnidx].value;
                            for(s32 slen =0 ;slen < SignalLen;slen++)
                            {
                                E2EData[realLen] = (value>>(8*slen))&0xff;
                                realLen++;
                            }
                        }
                    }
                    
                    it->second[e2eidx][datalen-2].value = crc8_calc(E2EData,realLen,0X11D);
                    set_signal_value(&it->second[e2eidx][datalen-2],AData->FData);//,(int)AData->FActualPayloadLength);
                }
            
                // if(it->second.size()!=0)
                // {
                //     it->second[0][0].AMsg = *AData;
                // }
            }
        }
	}
    }
}

bool Create_SocketServer(const char* ip,const int port,int backlog,bool is_tcp)
{
	#if defined(__linux__)
	int opt = 1; 
    if(is_tcp)
    {
	if(!init_tcp_socket(SocketServer,opt))
		return false;
    }
    else
    {
        if(!init_udp_socket(SocketServer,opt))
		return false;
    }
	if(!bind_socket(SocketServer,ip,port))
		return false;
    if(is_tcp)
    {
	    if(!listen_socket(SocketServer,backlog))
		    return false;
    }
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

string get_time(const char* hwindex)
{
    //获取系统时间戳
	char NowTime[50] ={0}; 
	time_t timeReal;
	time(&timeReal);
	timeReal = timeReal + 8*3600;
	tm* t = gmtime(&timeReal); 
	sprintf(NowTime,"%d-%02d-%02d_%02d_%02d_%02d_%s.asc", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,hwindex); 
	return NowTime;
}
int addrlen = sizeof(sockaddr_in);

void* receiveData(void* server_fd) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    while (true) {
            int valread = recvfrom(*((int*)server_fd), buffer, 1024, 0, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if(valread >= 302)
            {
                TLibFlexRayHW AMsg = *((PLibFlexRayHW)buffer);
                map<uint64_t,vector<vector<signal_parse>>>::iterator it = MappingTable[AMsg.HWIdx][AMsg.AMsg.FIdxChn][0][0].begin();
                for (;it!=MappingTable[AMsg.HWIdx][AMsg.AMsg.FIdxChn][0][0].end();it++)
                {
                    int Fslotid = (u16)((it->first>>48)&0xffff);
                    u8 baseCycle = (u8)((it->first>>40)&0xff);
                    u8 repCycle = (u8)((it->first>>32)&0xff);
                    if(AMsg.AMsg.FSlotId == Fslotid && AMsg.AMsg.FCycleNumber%repCycle == baseCycle)
                    {
                        if(it->second.size()!=0)
                            it->second[0][0].AMsg = AMsg.AMsg;
                        break;
                    }
                }
                tsflexray_transmit_async(HandleList[AMsg.HWIdx],&AMsg.AMsg);
            }
            else if (valread >= 80)
            {
                TLibCANFDHW AMsg = *((PLibCANFDHW)buffer);
                if(AMsg.CyclcTime ==0)
                {
                    tscan_transmit_canfd_async(HandleList[AMsg.HWIdx],&AMsg.AMsg);
                }
                else{
                    tscan_add_cyclic_msg_canfd(HandleList[AMsg.HWIdx],&AMsg.AMsg,AMsg.CyclcTime*1000);
                }
                
            }
            
    }
}
void config_bus(vector<map<uint32_t,map<uint32_t,vector<map<uint64_t, vector<vector<signal_parse>>>>>>> MappingTable)
{
    for(int i=0;i<MappingTable.size();i++)
    {
        for(int chnidx=0;chnidx<MaxCount;chnidx++)
        {
            for(int bustype=0;bustype<BUSTYPES;bustype++)
            {
                switch (bustype)
                {
                case 0:
                    if(MappingTable[i][chnidx][bustype].size()!=0)
                    {
                        InitFlexrayNode(HandleList[i],chnidx,MappingTable[i][chnidx][bustype][0]);
                        tsflexray_register_event_flexray_whandle(HandleList[i],OnFlexray);
                        // tsflexray_register_event_flexray_whandle(HandleList[i],On_tx_rxFlexray);
                        tsflexray_register_pretx_event_flexray_whandle(HandleList[i],PreTxFlexray);
                        tsflexray_start_net(HandleList[i],chnidx,1000);
                    }
                    break;
                case 1:
                    if(MappingTable[i][chnidx][bustype].size()!=0)
                    {
                        
                        ini_canbus(HandleList[i],chnidx,MappingTable[i][chnidx][bustype][0],OnPreCAN);
                    }
                    break;
                case 2:
                    if(MappingTable[i][chnidx][bustype].size()!=0)
                    {

                    }
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    initialize_lib_tscan(true,false,false);

    if(!Create_SocketServer(flexraysrcip.c_str(),flexraysrcport,1,false))
    {
        cout<<"udp create error"<<endl;
        return -1;
    }
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receiveData, (void*)&SocketServer);

    dstaddr.sin_family = AF_INET;
    dstaddr.sin_addr.s_addr = inet_addr(flexraydstip.c_str());
    dstaddr.sin_port = htons(flexraydstport);
    
    // if(argc<2)
    //     {
    //         cout<<"input error"<<endl;
    //         return -1;
    //     }
    if(Open_ini("config1.ini"))
    {
        cout<<"ini file open successed"<<endl;
        if(!Read_ini_Config(config))
        {
            cout<<"hw open error"<<endl;
            return -5;
        }
        Read_ini(config,MappingTable);
        config_bus(MappingTable);
    }
    else
    {
        cout<<"ini file find error"<<endl;
        return -2;
    }
    TLibCANFDHW ACANMSG;
    TLibFlexRayHW AFRMsg;
    while(1)
    {
        for(int i=0;i<HandleList.size();i++)
        {
            TLibFlexRay AMsg[1];
            s32 AMsgSize =1;
            tsfifo_receive_flexray_msgs(HandleList[i],AMsg,&AMsgSize,0xff,1);
            if(AMsgSize!=0 )
            {
                if(AMsg->FCCType==0 && AMsg->FSlotId != 65535)
                {
                    AFRMsg.HWIdx = i;
                    AFRMsg.AMsg = AMsg[0];
                    
                    udp_send_socket(SocketServer,(char*)(&AFRMsg),sizeof(TLibFlexRayHW),(sockaddr*)&dstaddr);
                    if(ISSaveBLF)
                    {
                        tslog_write_flexray(blf_BLHANDLEList[i],&AFRMsg.AMsg);
                    }
                }
            }
            TLibCANFD ACANFDMsg[1];
            s32 ACANFDMsgSize =1;
            s32 ret = tsfifo_receive_canfd_msgs(HandleList[i],ACANFDMsg,&ACANFDMsgSize,0xff,1);
            if(ACANFDMsgSize!=0 )
            {
                
                ACANMSG.HWIdx = i;
                ACANMSG.CyclcTime = 0;
                ACANMSG.AMsg = ACANFDMsg[0];
                udp_send_socket(SocketServer,(char*)(&ACANMSG),sizeof(TLibCANFDHW),(sockaddr*)&dstaddr);
                if(ISSaveBLF)
                {
                    tslog_write_can(blf_BLHANDLEList[i],&ACANMSG.AMsg);
                }
            }
            if(ACANFDMsgSize==0 && AMsgSize==0)
                usleep(1);
            
        }
    }
    return 0;
}

