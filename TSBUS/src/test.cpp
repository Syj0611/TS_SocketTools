/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-08-17 01:12:57
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-18 00:13:16
 * @FilePath: /TSBUS_SocketTools/TSBUS/src/test.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
// #include"TSBUSConfig.cpp"
#pragma once
#include<iostream>
#include<cstdio>
#include<bitset>
#include"TSCANDef.hpp"
#include"TSMSGStruct.h"
#include <string.h>
using namespace std;

int main()
{
    printf("CANFDHWSTRUCT =  %d\r\n",sizeof(TLibCANFDHW));
    printf("LINHWSTRUCT =  %d\r\n",sizeof(TLibLINHW));
    printf("FRHWSTRUCT =  %d\r\n",sizeof(TLibFlexRayHW));
}