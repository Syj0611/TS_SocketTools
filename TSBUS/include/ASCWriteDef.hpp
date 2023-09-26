/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-07-01 20:04:56
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-21 03:24:59
 * @FilePath: \window_linux_Rep\c++\libASCLog\include\ASCWriteDef.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <fstream>
#include "TSCANDef.hpp"
#include<ctime>
const u8 DLC_DATA_BYTE_CNT[16] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 12, 16, 20, 24, 32, 48, 64
};

extern "C"
{
    
    s32 tslog_write_start(const char*logfile,FILE**file); /* optional function */
    
    s32 tslog_write_can(FILE*file,const PLibCANFD ACANFD);

    s32 tslog_write_lin(FILE*file,const PLibLIN ALIN);

    s32 tslog_write_flexray(FILE*file,const PLibFlexRay AFlexray);

    s32 tslog_write_end(FILE*file);
}