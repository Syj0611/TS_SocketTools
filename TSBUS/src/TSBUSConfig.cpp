/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-08-03 21:33:32
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-20 21:35:51
 * @FilePath: \window_linux_Rep\c++\TSFlexray\include\TSBUSConfig.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "TSCANDef.hpp"
#include<map>

#include<iostream>
#include<cstdio>
#include<bitset>
#include"./stdc++.h"
#include <string.h>
using namespace std;

#define CoolCode  0x31
#define NormalCode  0x1
#define OneSend  0xA9

typedef struct _signal_parse
{
    int32_t start_bit;
    int32_t bitlen;
    int32_t start_byte;
    int32_t byte_len;
    u8 is_intel;
    u8 is_unSigned;
    u16 dataid;
    u8 is_cntr;
    u8 errorcounter;
    u8 errorcrc;
    
    double factor;
    double offset;
    double value;
    
    TLibFlexRay AMsg;
    // char AMsg[302];
}signal_parse,*psignal_parse;

typedef struct _frame_data
{
    u8 FData[254];
    u8 is_wake;  
    vector<vector<signal_parse>> e2e_list;
    // char AMsg[302];
}frame_data,*pframe_data;

void get_real_signal(psignal_parse Signal)
{
    Signal->start_byte = (Signal->start_bit) / 8 ;
    int val_bit = Signal->start_bit % 8;
    if(Signal->is_intel)
    {
        val_bit = Signal->bitlen - (8 - val_bit);
        if(val_bit % 8 ==0)
            Signal->byte_len = val_bit / 8;
        else
            Signal->byte_len = (val_bit / 8) + 1;
    }
    else
    {
        val_bit = Signal->bitlen - (val_bit + 1);
        if(val_bit % 8 ==0)
            Signal->byte_len = val_bit / 8;
        else
            Signal->byte_len = (val_bit / 8) + 1;
    }
    Signal->byte_len +=1;
    Signal->start_bit = Signal->start_bit - Signal->start_byte * 8;
}


s32 calcrealstartbit(int startbit,int bitlen,int intel)
{
    int val_len = startbit % 8 +1;
    if(!intel)
    {
        if(bitlen<=val_len)
        {
            startbit = startbit - bitlen + 1;
        }
        else
        {
            //len 2                  //11  87                  
            int second_bit = (((startbit/8) +1)+ ((bitlen - val_len)/9 +1))*8 - 1;

            int second_bit_len = (bitlen - val_len)%8;
            if(second_bit_len == 0)
                startbit = second_bit - 7;
            else
                startbit = second_bit - second_bit_len + 1;
        }
    }
    else{
        startbit = startbit - bitlen + 1;
    }
    return 0;    
}

s32 InitFlexrayNode(size_t ADeviceHandle, int AChnIdx,PLibTrigger_def Frames,const int* FrameLength,const int len)
{
    TLibFlexray_controller_config	controllerConfig;
    //memset(&controllerConfig, 0, sizeof(TLibFlexray_controller_config));
    int channelIndex = AChnIdx;
    controllerConfig.NETWORK_MANAGEMENT_VECTOR_LENGTH = 8;
    controllerConfig.PAYLOAD_LENGTH_STATIC = 16;
    controllerConfig.LATEST_TX = 124;
    // __ prtc1Control
    controllerConfig.T_S_S_TRANSMITTER = 9;
    controllerConfig.CAS_RX_LOW_MAX = 87;
    controllerConfig.SPEED = 0;
    controllerConfig.WAKE_UP_SYMBOL_RX_WINDOW = 301;
    if (channelIndex == 0)
        controllerConfig.WAKE_UP_PATTERN = 43;
    else
        controllerConfig.WAKE_UP_PATTERN = 42;
    // __ prtc2Control
    controllerConfig.WAKE_UP_SYMBOL_RX_IDLE = 59;
    controllerConfig.WAKE_UP_SYMBOL_RX_LOW = 55;
    controllerConfig.WAKE_UP_SYMBOL_TX_IDLE = 180;
    controllerConfig.WAKE_UP_SYMBOL_TX_LOW = 60;
    // __ succ1Config
    controllerConfig.channelAConnectedNode = 1;// 是否启用通道A,0不启动，1启动
    controllerConfig.channelBConnectedNode = 1;// 是否启用通道B,0不启动，1启动
    controllerConfig.channelASymbolTransmitted = 1;// 是否启用通道A的符号传输功能,0不启动，1启动
    controllerConfig.channelBSymbolTransmitted = 1;// 是否启用通道B的符号传输功能,0不启动，1启动
    controllerConfig.ALLOW_HALT_DUE_TO_CLOCK = 1;
    controllerConfig.SINGLE_SLOT_ENABLED = 0;// FALSE_0, TRUE_1
    controllerConfig.wake_up_idx = 0;// 唤醒通道选择， 0_通道A， 1 通道B
    controllerConfig.ALLOW_PASSIVE_TO_ACTIVE = 2;
    controllerConfig.COLD_START_ATTEMPTS = 10;
    controllerConfig.synchFrameTransmitted = 1;// 本节点是否需要发送同步报文
    controllerConfig.startupFrameTransmitted = 1;// 本节点是否需要发送启动报文
                // __ succ2Config
    controllerConfig.LISTEN_TIMEOUT = 401202;
    controllerConfig.LISTEN_NOISE = 2;//2_16
                // __ succ3Config
    controllerConfig.MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE = 10;
    controllerConfig.MAX_WITHOUT_CLOCK_CORRECTION_FATAL = 14;
    //uint8_t REVERS0;//内存对齐
    // __ gtuConfig
    // __ gtu01Config
    controllerConfig.MICRO_PER_CYCLE = 200000;
    // __ gtu02Config
    controllerConfig.Macro_Per_Cycle = 5000;
    controllerConfig.SYNC_NODE_MAX = 8;
    //uint8_t REVERS1;//内存对齐
    // __ gtu03Config
    controllerConfig.MICRO_INITIAL_OFFSET_A = 31;
    controllerConfig.MICRO_INITIAL_OFFSET_B = 31;
    controllerConfig.MACRO_INITIAL_OFFSET_A = 11;
    controllerConfig.MACRO_INITIAL_OFFSET_B = 11;
    // __ gtu04Config
    controllerConfig.N_I_T = 44;
    controllerConfig.OFFSET_CORRECTION_START = 4981;
    // __ gtu05Config
    controllerConfig.DELAY_COMPENSATION_A = 1;
    controllerConfig.DELAY_COMPENSATION_B = 1;
    controllerConfig.CLUSTER_DRIFT_DAMPING = 2;
    controllerConfig.DECODING_CORRECTION = 48;
    // __ gtu06Config
    controllerConfig.ACCEPTED_STARTUP_RANGE = 212;
    controllerConfig.MAX_DRIFT = 601;
    // __ gtu07Config
    controllerConfig.STATIC_SLOT = 61;
    controllerConfig.NUMBER_OF_STATIC_SLOTS = 60;
    // __ gtu08Config
    controllerConfig.MINISLOT = 10;
    //uint8_t REVERS2;//内存对齐
    controllerConfig.NUMBER_OF_MINISLOTS = 129;
    // __ gtu09Config
    controllerConfig.DYNAMIC_SLOT_IDLE_PHASE = 0;
    controllerConfig.ACTION_POINT_OFFSET = 9;
    controllerConfig.MINISLOT_ACTION_POINT_OFFSET = 3;
    //uint8_t REVERS3;//内存对齐
    // __ gtu10Config
    controllerConfig.OFFSET_CORRECTION_OUT = 378;
    controllerConfig.RATE_CORRECTION_OUT = 601;
    // __ gtu11Config
    controllerConfig.EXTERN_OFFSET_CORRECTION = 0;
    controllerConfig.EXTERN_RATE_CORRECTION = 0;
    controllerConfig.config_byte1 = 1;  //启动桥接功能，启用接收FIFO，启动终端电阻
    controllerConfig.config_byte = 0xbF;  //启动桥接功能，启用接收FIFO，启动终端电阻
    return tsflexray_set_controller_frametrigger(ADeviceHandle, channelIndex, &controllerConfig, &FrameLength[0], len, Frames, len, 1000);
}


s32 InitFlexrayNode(size_t ADeviceHandle, map<uint32_t,map<uint64_t,vector<vector<signal_parse>>>>& ChnList)
{
    int ret = -1;
    TLibFlexray_controller_config	controllerConfig;
    controllerConfig.NETWORK_MANAGEMENT_VECTOR_LENGTH = 8;
        controllerConfig.PAYLOAD_LENGTH_STATIC = 16;
        controllerConfig.LATEST_TX = 124;
        // __ prtc1Control
        controllerConfig.T_S_S_TRANSMITTER = 9;
        controllerConfig.CAS_RX_LOW_MAX = 87;
        controllerConfig.SPEED = 0;
        controllerConfig.WAKE_UP_SYMBOL_RX_WINDOW = 301;
        
        // __ prtc2Control
        controllerConfig.WAKE_UP_SYMBOL_RX_IDLE = 59;
        controllerConfig.WAKE_UP_SYMBOL_RX_LOW = 55;
        controllerConfig.WAKE_UP_SYMBOL_TX_IDLE = 180;
        controllerConfig.WAKE_UP_SYMBOL_TX_LOW = 60;
        // __ succ1Config
        controllerConfig.channelAConnectedNode = 1;// 是否启用通道A,0不启动，1启动
        controllerConfig.channelBConnectedNode = 1;// 是否启用通道B,0不启动，1启动
        controllerConfig.channelASymbolTransmitted = 1;// 是否启用通道A的符号传输功能,0不启动，1启动
        controllerConfig.channelBSymbolTransmitted = 1;// 是否启用通道B的符号传输功能,0不启动，1启动
        controllerConfig.ALLOW_HALT_DUE_TO_CLOCK = 1;
        controllerConfig.SINGLE_SLOT_ENABLED = 0;// FALSE_0, TRUE_1
        controllerConfig.wake_up_idx = 0;// 唤醒通道选择， 0_通道A， 1 通道B
        controllerConfig.ALLOW_PASSIVE_TO_ACTIVE = 2;
        controllerConfig.COLD_START_ATTEMPTS = 10;
        controllerConfig.synchFrameTransmitted = 1;// 本节点是否需要发送同步报文
        controllerConfig.startupFrameTransmitted = 1;// 本节点是否需要发送启动报文
                    // __ succ2Config
        controllerConfig.LISTEN_TIMEOUT = 401202;
        controllerConfig.LISTEN_NOISE = 2;//2_16
                    // __ succ3Config
        controllerConfig.MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE = 10;
        controllerConfig.MAX_WITHOUT_CLOCK_CORRECTION_FATAL = 14;
        //uint8_t REVERS0;//内存对齐
        // __ gtuConfig
        // __ gtu01Config
        controllerConfig.MICRO_PER_CYCLE = 200000;
        // __ gtu02Config
        controllerConfig.Macro_Per_Cycle = 5000;
        controllerConfig.SYNC_NODE_MAX = 8;
        //uint8_t REVERS1;//内存对齐
        // __ gtu03Config
        controllerConfig.MICRO_INITIAL_OFFSET_A = 31;
        controllerConfig.MICRO_INITIAL_OFFSET_B = 31;
        controllerConfig.MACRO_INITIAL_OFFSET_A = 11;
        controllerConfig.MACRO_INITIAL_OFFSET_B = 11;
        // __ gtu04Config
        controllerConfig.N_I_T = 44;
        controllerConfig.OFFSET_CORRECTION_START = 4981;
        // __ gtu05Config
        controllerConfig.DELAY_COMPENSATION_A = 1;
        controllerConfig.DELAY_COMPENSATION_B = 1;
        controllerConfig.CLUSTER_DRIFT_DAMPING = 2;
        controllerConfig.DECODING_CORRECTION = 48;
        // __ gtu06Config
        controllerConfig.ACCEPTED_STARTUP_RANGE = 212;
        controllerConfig.MAX_DRIFT = 601;
        // __ gtu07Config
        controllerConfig.STATIC_SLOT = 61;
        controllerConfig.NUMBER_OF_STATIC_SLOTS = 60;
        // __ gtu08Config
        controllerConfig.MINISLOT = 10;
        //uint8_t REVERS2;//内存对齐
        controllerConfig.NUMBER_OF_MINISLOTS = 129;
        // __ gtu09Config
        controllerConfig.DYNAMIC_SLOT_IDLE_PHASE = 0;
        controllerConfig.ACTION_POINT_OFFSET = 9;
        controllerConfig.MINISLOT_ACTION_POINT_OFFSET = 3;
        //uint8_t REVERS3;//内存对齐
        // __ gtu10Config
        controllerConfig.OFFSET_CORRECTION_OUT = 378;
        controllerConfig.RATE_CORRECTION_OUT = 601;
        // __ gtu11Config
        controllerConfig.EXTERN_OFFSET_CORRECTION = 0;
        controllerConfig.EXTERN_RATE_CORRECTION = 0;
        controllerConfig.config_byte1 = 1;  //启动桥接功能，启用接收FIFO，启动终端电阻
        controllerConfig.config_byte = 0xbF;  //启动桥接功能，启用接收FIFO，启动终端电阻
        controllerConfig.WAKE_UP_PATTERN = 43;
    //memset(&controllerConfig, 0, sizeof(TLibFlexray_controller_config));
    int FrameLength[124] = {0};
    for(int i = 0; i < ChnList.size(); i++)
    {
        
        TLibTrigger_def Trigger_def[124] = {0};
        map<uint64_t,vector<vector<signal_parse>>>::iterator it = ChnList[i].begin();
        s32 len = ChnList[i].size();
        for (int idx=0;it != ChnList[i].end() && idx<124; it++,idx++)
        {
            TLibTrigger_def Trigger ;
            Trigger.slot_id = (u16)((it->first>>48)&0xffff);
            Trigger.cycle_code = (u8)((it->first>>40)&0xff)+(u8)((it->first>>32)&0xff);
            Trigger.frame_idx = idx;
            Trigger.rev =0;
            Trigger_def[idx] = Trigger;
            // Trigger_def[idx].slot_id = (u16)((it->first>>48)&0xffff);
            // Trigger_def[idx].cycle_code = (u8)((it->first>>40)&0xff)+(u8)((it->first>>32)&0xff);
            // Trigger_def[idx].frame_idx = idx;
            // Trigger_def[idx].rev =0;
            FrameLength[idx] = (u8)(((it->first))&0xff);
            if(Trigger_def[idx].slot_id == Trigger_def[0].slot_id)
            {
                Trigger_def[idx].config_byte = CoolCode;
            }
            else if(Trigger_def[idx].slot_id<61)
            {
                Trigger_def[idx].config_byte = NormalCode;
            }
            else 
                Trigger_def[idx].config_byte = OneSend;

        }
        ret = tsflexray_set_controller_frametrigger(ADeviceHandle, i, &controllerConfig, FrameLength, len, Trigger_def, len, 1000);

        if (ret != 0)
            return ret;
    }
    return ret;
}

s32 InitFlexrayNode(size_t ADeviceHandle, int AChnidx, map<uint64_t, frame_data>& ChnList)
{
    int ret = -1;
    TLibFlexray_controller_config	controllerConfig;
    controllerConfig.NETWORK_MANAGEMENT_VECTOR_LENGTH = 8;
        controllerConfig.PAYLOAD_LENGTH_STATIC = 16;
        controllerConfig.LATEST_TX = 124;
        // __ prtc1Control
        controllerConfig.T_S_S_TRANSMITTER = 9;
        controllerConfig.CAS_RX_LOW_MAX = 87;
        controllerConfig.SPEED = 0;
        controllerConfig.WAKE_UP_SYMBOL_RX_WINDOW = 301;
        
        // __ prtc2Control
        controllerConfig.WAKE_UP_SYMBOL_RX_IDLE = 59;
        controllerConfig.WAKE_UP_SYMBOL_RX_LOW = 55;
        controllerConfig.WAKE_UP_SYMBOL_TX_IDLE = 180;
        controllerConfig.WAKE_UP_SYMBOL_TX_LOW = 60;
        // __ succ1Config
        controllerConfig.channelAConnectedNode = 1;// 是否启用通道A,0不启动，1启动
        controllerConfig.channelBConnectedNode = 1;// 是否启用通道B,0不启动，1启动
        controllerConfig.channelASymbolTransmitted = 1;// 是否启用通道A的符号传输功能,0不启动，1启动
        controllerConfig.channelBSymbolTransmitted = 1;// 是否启用通道B的符号传输功能,0不启动，1启动
        controllerConfig.ALLOW_HALT_DUE_TO_CLOCK = 1;
        controllerConfig.SINGLE_SLOT_ENABLED = 0;// FALSE_0, TRUE_1
        controllerConfig.wake_up_idx = 0;// 唤醒通道选择， 0_通道A， 1 通道B
        controllerConfig.ALLOW_PASSIVE_TO_ACTIVE = 2;
        controllerConfig.COLD_START_ATTEMPTS = 10;
        controllerConfig.synchFrameTransmitted = 1;// 本节点是否需要发送同步报文
        controllerConfig.startupFrameTransmitted = 1;// 本节点是否需要发送启动报文
                    // __ succ2Config
        controllerConfig.LISTEN_TIMEOUT = 401202;
        controllerConfig.LISTEN_NOISE = 2;//2_16
                    // __ succ3Config
        controllerConfig.MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE = 10;
        controllerConfig.MAX_WITHOUT_CLOCK_CORRECTION_FATAL = 14;
        //uint8_t REVERS0;//内存对齐
        // __ gtuConfig
        // __ gtu01Config
        controllerConfig.MICRO_PER_CYCLE = 200000;
        // __ gtu02Config
        controllerConfig.Macro_Per_Cycle = 5000;
        controllerConfig.SYNC_NODE_MAX = 8;
        //uint8_t REVERS1;//内存对齐
        // __ gtu03Config
        controllerConfig.MICRO_INITIAL_OFFSET_A = 31;
        controllerConfig.MICRO_INITIAL_OFFSET_B = 31;
        controllerConfig.MACRO_INITIAL_OFFSET_A = 11;
        controllerConfig.MACRO_INITIAL_OFFSET_B = 11;
        // __ gtu04Config
        controllerConfig.N_I_T = 44;
        controllerConfig.OFFSET_CORRECTION_START = 4981;
        // __ gtu05Config
        controllerConfig.DELAY_COMPENSATION_A = 1;
        controllerConfig.DELAY_COMPENSATION_B = 1;
        controllerConfig.CLUSTER_DRIFT_DAMPING = 2;
        controllerConfig.DECODING_CORRECTION = 48;
        // __ gtu06Config
        controllerConfig.ACCEPTED_STARTUP_RANGE = 212;
        controllerConfig.MAX_DRIFT = 601;
        // __ gtu07Config
        controllerConfig.STATIC_SLOT = 61;
        controllerConfig.NUMBER_OF_STATIC_SLOTS = 60;
        // __ gtu08Config
        controllerConfig.MINISLOT = 10;
        //uint8_t REVERS2;//内存对齐
        controllerConfig.NUMBER_OF_MINISLOTS = 129;
        // __ gtu09Config
        controllerConfig.DYNAMIC_SLOT_IDLE_PHASE = 0;
        controllerConfig.ACTION_POINT_OFFSET = 9;
        controllerConfig.MINISLOT_ACTION_POINT_OFFSET = 3;
        //uint8_t REVERS3;//内存对齐
        // __ gtu10Config
        controllerConfig.OFFSET_CORRECTION_OUT = 378;
        controllerConfig.RATE_CORRECTION_OUT = 601;
        // __ gtu11Config
        controllerConfig.EXTERN_OFFSET_CORRECTION = 0;
        controllerConfig.EXTERN_RATE_CORRECTION = 0;
        controllerConfig.config_byte1 = 1;  //启动桥接功能，启用接收FIFO，启动终端电阻
        controllerConfig.config_byte = 0xbF;  //启动桥接功能，启用接收FIFO，启动终端电阻
        controllerConfig.WAKE_UP_PATTERN = 43;
    //memset(&controllerConfig, 0, sizeof(TLibFlexray_controller_config));
        int FrameLength[124] = {0};

        TLibTrigger_def Trigger_def[124] = {0};
        map<uint64_t,frame_data>::iterator it = ChnList.begin();
        s32 len = ChnList.size();
        for (int idx=0;it != ChnList.end() && idx<124; it++,idx++)
        {
            TLibTrigger_def Trigger ;
            Trigger.slot_id = (u16)((it->first>>48)&0xffff);
            Trigger.cycle_code = (u8)((it->first>>40)&0xff)+(u8)((it->first>>32)&0xff);
            Trigger.frame_idx = idx;
            Trigger.rev =0;
            Trigger_def[idx] = Trigger;
            // Trigger_def[idx].slot_id = (u16)((it->first>>48)&0xffff);
            // Trigger_def[idx].cycle_code = (u8)((it->first>>40)&0xff)+(u8)((it->first>>32)&0xff);
            // Trigger_def[idx].frame_idx = idx;
            // Trigger_def[idx].rev =0;
            FrameLength[idx] = (u8)(((it->first))&0xff);
            if(Trigger_def[idx].slot_id == Trigger_def[0].slot_id)
            {
                Trigger_def[idx].config_byte = CoolCode;
            }
            else if(Trigger_def[idx].slot_id<61)
            {
                Trigger_def[idx].config_byte = NormalCode;
            }
            else 
                Trigger_def[idx].config_byte = OneSend;

        }
        ret = tsflexray_set_controller_frametrigger(ADeviceHandle, AChnidx, &controllerConfig, FrameLength, len, Trigger_def, len, 1000);
        return ret;
}

// void set_signal_value(psignal_parse signal, uint8_t* data)
// {

//     uint32_t byte_offset = signal->start_bit / 8;
//     uint32_t bit_offset = signal->start_bit % 8;
//     uint32_t bit_len = signal->bitlen;
//     uint32_t byte_len = (bit_offset + bit_len + 7) / 8;
//     uint8_t* p = data + byte_offset;
//     uint32_t int_value = (uint32_t)((signal->value - signal->offset) / signal->factor);
//     if (signal->is_intel)
//     {
//         uint32_t j;
//         uint32_t tmp = 0;
//         for (j = 0; j < bit_len; j++)
//         {
//             tmp <<= 1;
//             tmp |= (int_value & (1 << j)) >> j;
//         }
//         int_value = tmp;
//     }
//     if (!signal->is_unSigned && (int_value & (1 << (bit_len - 1))))
//     {
//         int_value |= ~((1 << bit_len) - 1);
//     }
//     //int_value <<= bit_offset;
//     uint32_t i;
//     for (i = 0; i < byte_len; i++)
//     {
//         *(p + i) &= ~(0xff << bit_offset);
//         *(p + i) |= ((int_value >> (8 * i)) & 0xff) << bit_offset;
//     }
// }

// double get_signal_value( psignal_parse signal,uint8_t* data)
// {
    
//     uint32_t byte_offset = signal->start_bit / 8;
//     uint32_t bit_offset = signal->start_bit % 8;
//     uint32_t bit_len = signal->bitlen;
//     uint32_t byte_len = (bit_offset + bit_len + 7) / 8;
//     uint8_t* p = data + byte_offset;
//     uint32_t value = 0;
//     uint32_t i;
//     for (i = 0; i < byte_len; i++)
//     {
//         value <<= 8;
//         value |= *(p + i);
//     }
//     value >>= bit_offset;
//     value &= (1 << bit_len) - 1;
//     if (signal->is_intel)
//     {
//         uint32_t j;
//         uint32_t tmp = 0;
//         for (j = 0; j < bit_len; j++)
//         {
//             tmp <<= 1;
//             tmp |= (value & (1 << j)) >> j;
//         }
//         value = tmp;
//     }
//     if (!signal->is_unSigned && (value & (1 << (bit_len - 1))))
//     {
//         value |= ~((1 << bit_len) - 1);
//     }
//     signal->value = (((double)value) * signal->factor + signal->offset);
//     return signal->value;
// }
// double  get_signal_value(signal_parse* signal,uint8_t* data, int len)
// {
//     string data_string;
//     for (int i = 0; i<len; i++)
//     {
//         bitset<8> a(data[i]);
//         string temp = a.to_string();
//         if (signal->is_intel)
//             reverse(temp.begin(), temp.end());
//         data_string += temp;
//     }
//     string str;
//     if (signal->is_intel)
//     {
//         str = data_string.substr(signal->start_bit, signal->bitlen);
//         reverse(str.begin(), str.end());
//     }
//     else
//     {
//         str = data_string.substr(((signal->start_bit / 8 + 1) * 8 - (signal->start_bit % 8 + 1) - signal->bitlen + 1), signal->bitlen);
//     }

//     uint64_t value = bitset<64>(str).to_ullong();
//     if (signal->is_unSigned)
//         return double(value) * signal->factor + signal->offset;
//     if ((value >> (signal->bitlen - 1)) & 1)
//     {
//         value = (((~value) & (((1 << (signal->bitlen - 1)) - 1))) + 1);
//         return -double(value) * signal->factor + signal->offset;
//     }
//     signal->value = double(value) * signal->factor + signal->offset;
//     return signal->value;
// }
// int32_t  set_signal_value(signal_parse* signal,uint8_t* data, uint32_t len)
// {
//     string data_string;
//     int index = 0;
//     for (int i = 0; i<len; i++)
//     {
//         bitset<8> a(data[i]);
//         string temp = a.to_string();
//         if (signal->is_intel)
//             reverse(temp.begin(), temp.end());
//         data_string += temp;
//         index = i;
//     }
//     signal->value = (signal->value - signal->offset) / signal->factor;
//     bitset<64>b(signal->value);

//     string temp = b.to_string().substr(64 - signal->bitlen, signal->bitlen);
//     try {
//         if (signal->is_intel)
//         {
//             reverse(temp.begin(), temp.end());
//             data_string.replace(signal->start_bit, signal->bitlen, temp);
//         }
//         else
//         {
//             data_string.replace(((signal->start_bit / 8 + 1) * 8 - (signal->start_bit % 8 + 1) - signal->bitlen + 1), signal->bitlen, temp);
//         }
//     }
//     catch (exception& e)
//     {
//         return 0xff;
//     }
//     for (index; index >= 0; index--)
//     {
//         string data_temp = data_string.substr(index * 8, 8);
//         if (signal->is_intel)
//             reverse(data_temp.begin(), data_temp.end());
//         data[index] = (uint8_t )strtoul(data_temp.c_str(), NULL, 2);
//     }
//     return 0x0;
// }

// double get_signal_value(psignal_parse Signal,uint8_t* data, int len)
// {
//     uint64_t value =0;
//     string strvalue;
//     if(Signal->is_intel)
//     {
//         for(int i = 0; i < len; i++)
//         {
//             if(data[i]>=0x80)
//             {
//                 bitset<8>a(data[i]);
//                 string temp = a.to_string();
//                 reverse(temp.begin(), temp.end());
//                 strvalue += temp;
//             }
//             else
//             {
//                 bitset<8>temp((data[i] | 0x80));
//                 string tempstr = temp.to_string();
//                 tempstr[0] = '0';
//                 reverse(tempstr.begin(), tempstr.end());
//                 strvalue += tempstr;
//             }
//         }

//         int startidx = Signal->start_bit;
//         string temp = strvalue.substr(startidx, Signal->bitlen);
//         reverse(temp.begin(), temp.end());
//         Signal->value = strtoll(temp.c_str(),NULL,2);
//         return Signal->value;
//     }
//     else
//     {
//         for(int i = 0; i < len; i++)
//         {
            
//             if(data[i]>=0x80)
//             {
//                 bitset<8>temp(data[i]);
                
//                 strvalue += temp.to_string();
//             }
//             else
//             {
//                 bitset<8>temp((data[i] | 0x80));
//                 string tempstr = temp.to_string();
//                 tempstr[0] = '0';
//                 strvalue += tempstr;
//             }
//         }
//         int startidx = ((Signal->start_bit/8 + 1)*8 - 1) - (Signal->start_bit%8);
//         string temp = strvalue.substr(startidx, Signal->bitlen);
//         Signal->value = strtoll(temp.c_str(),NULL,2);
//         return Signal->value;
//     }
// }

// void set_signal_value(psignal_parse Signal,uint8_t* data, int len)
// {
//     uint64_t value = (uint64_t)((Signal->value - Signal->offset)/Signal->factor)&((1<<Signal->bitlen)-1);
//     bitset<64> bs(value);
//     string value_str = bs.to_string();
//     if(value_str.size()>Signal->bitlen)
//         value_str = value_str.substr(value_str.size()-Signal->bitlen);
//     string strvalue ;
//     if(Signal->is_intel)
//     {
//         for(int i = 0; i < len; i++)
//         {
//             if(data[i]>=0x80)
//             {
//                 bitset<8>a(data[i]);
//                 string temp = a.to_string();
//                 reverse(temp.begin(), temp.end());
//                 strvalue += temp;
//             }
//             else
//             {
//                 bitset<8>temp((data[i] | 0x80));
//                 string tempstr = temp.to_string();
//                 tempstr[0] = '0';
//                 reverse(tempstr.begin(), tempstr.end());
//                 strvalue += tempstr;
//             }
//         }
//         int startidx = Signal->start_bit;
//         int endidx = startidx + Signal->bitlen - 1;
//         //120  12 13 14 15
//         strvalue = strvalue.substr(0,startidx) + value_str + strvalue.substr(endidx + 1);
//         // cout<<"value is "<<strvalue<<endl;
//         for(int i = 0; i < len; i++)
//         {
//             string data_temp = strvalue.substr(i * 8, 8);
//             reverse(data_temp.begin(), data_temp.end());
//             data[i] = (uint8_t)strtoul(data_temp.c_str(), NULL, 2);
//         }
//     }
//     else
//     {
//         for(int i = 0; i < len; i++)
//         {
            
//             if(data[i]>=0x80)
//             {
//                 bitset<8>temp(data[i]);
                
//                 strvalue += temp.to_string();
//             }
//             else
//             {
//                 bitset<8>temp((data[i] | 0x80));
//                 string tempstr = temp.to_string();
//                 tempstr[0] = '0';
                
//                 strvalue += tempstr;
//             }
//         }
//         int startidx = ((Signal->start_bit/8 + 1)*8 - 1) - (Signal->start_bit%8);
//         int endidx = startidx + Signal->bitlen - 1;
//         strvalue = strvalue.substr(0,startidx) + value_str + strvalue.substr(endidx + 1);
//         // cout<<"value is "<<strvalue<<endl;
//         for(int i = 0; i < len; i++)
//         {
//             data[i] = (uint8_t)strtol(strvalue.substr(i*8,8).c_str(),NULL,2);
//         }
//     }

// }

double get_signal_value(psignal_parse Signal,uint8_t* data)
{
    uint64_t value =0;
    string strvalue;
    if(Signal->is_intel)
    {
        for(int i = Signal->start_byte; i < Signal->start_byte + Signal->byte_len; i++)
        {
            if(data[i]>=0x80)
            {
                bitset<8>a(data[i]);
                string temp = a.to_string();
                reverse(temp.begin(), temp.end());
                strvalue += temp;
            }
            else
            {
                bitset<8>temp((data[i] | 0x80));
                string tempstr = temp.to_string();
                tempstr[0] = '0';
                reverse(tempstr.begin(), tempstr.end());
                strvalue += tempstr;
            }
        }

        int startidx = Signal->start_bit;
        string temp = strvalue.substr(startidx, Signal->bitlen);
        //reverse(temp.begin(), temp.end());
        Signal->value = strtoll(temp.c_str(),NULL,2);
        return Signal->value;
    }
    else
    {
        for(int i = Signal->start_byte; i < Signal->start_byte + Signal->byte_len; i++)
        {
            
            if(data[i]>=0x80)
            {
                bitset<8>temp(data[i]);
                
                strvalue += temp.to_string();
            }
            else
            {
                bitset<8>temp((data[i] | 0x80));
                string tempstr = temp.to_string();
                tempstr[0] = '0';
                strvalue += tempstr;
            }
        }
        int startidx = ((Signal->start_bit/8 + 1)*8 - 1) - (Signal->start_bit%8);
        string temp = strvalue.substr(startidx, Signal->bitlen);
        Signal->value = strtoll(temp.c_str(),NULL,2);
        return Signal->value;
    }
}

void set_signal_value(psignal_parse Signal,uint8_t* data)
{
    uint64_t value = (uint64_t)((Signal->value - Signal->offset)/Signal->factor)&((1<<Signal->bitlen)-1);
    bitset<64> bs(value);
    string value_str = bs.to_string();
    if(value_str.size()>Signal->bitlen)
        value_str = value_str.substr(value_str.size()-Signal->bitlen);
    string strvalue ;
    if(Signal->is_intel)
    {
        for(int i = Signal->start_byte; i < Signal->start_byte + Signal->byte_len; i++)
        {
            if(data[i]>=0x80)
            {
                bitset<8>a(data[i]);
                string temp = a.to_string();
                reverse(temp.begin(), temp.end());
                strvalue += temp;
            }
            else
            {
                bitset<8>temp((data[i] | 0x80));
                string tempstr = temp.to_string();
                tempstr[0] = '0';
                reverse(tempstr.begin(), tempstr.end());
                strvalue += tempstr;
            }
        }
        int startidx = Signal->start_bit;
        int endidx = startidx + Signal->bitlen - 1;
        //120  12 13 14 15
        strvalue = strvalue.substr(0,startidx) + value_str + strvalue.substr(endidx + 1);
        // cout<<"value is "<<strvalue<<endl;
        for(int i = 0; i < Signal->byte_len; i++)
        {
            string data_temp = strvalue.substr(i * 8, 8);
            reverse(data_temp.begin(), data_temp.end());
            data[Signal->start_byte+ i] = (uint8_t)strtoul(data_temp.c_str(), NULL, 2);
        }
    }
    else
    {
        for(int i = Signal->start_byte; i < Signal->start_byte + Signal->byte_len; i++)
        {
            
            if(data[i]>=0x80)
            {
                bitset<8>temp(data[i]);
                
                strvalue += temp.to_string();
            }
            else
            {
                bitset<8>temp((data[i] | 0x80));
                string tempstr = temp.to_string();
                tempstr[0] = '0';
                
                strvalue += tempstr;
            }
        }
        int startidx = ((Signal->start_bit/8 + 1)*8 - 1) - (Signal->start_bit%8);
        int endidx = startidx + Signal->bitlen - 1;
        strvalue = strvalue.substr(0,startidx) + value_str + strvalue.substr(endidx + 1);
        // cout<<"value is "<<strvalue<<endl;
        for(int i = 0; i < Signal->byte_len; i++)
        {
            data[Signal->start_byte+ i ] = (uint8_t)strtol(strvalue.substr(i*8,8).c_str(),NULL,2);
        }
    }

}
