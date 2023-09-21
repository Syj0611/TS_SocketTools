#include "TSCANDef.hpp"
typedef struct _TLibCANFDHW{
    int HWIdx;
    float CyclcTime; //-1 stop msg -1 stop net
    int LogFlag; //0 start 1 stop
    TLibCANFD AMsg;
} TLibCANFDHW,*PLibCANFDHW;

typedef struct _TLibFlexRayHW{
    int HWIdx;
    //在udp报文里发送暂停启动的属性
    int StopNet;//0 start, 1 stop
    int LogFlag; //0 start 1 error
    TLibFlexRay AMsg;
} TLibFlexRayHW,*PLibFlexRayHW;

typedef struct _TLibLINHW{
    int HWIdx;
    TLibLIN AMsg;
} TLibLINHW,*PLibLINHW;