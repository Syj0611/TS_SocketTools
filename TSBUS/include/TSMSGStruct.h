#include "TSCANDef.hpp"
typedef struct _TLibCANFDHW{
    int HWIdx;
    float CyclcTime;
    TLibCANFD AMsg;
} TLibCANFDHW,*PLibCANFDHW;

typedef struct _TLibFlexRayHW{
    int HWIdx;
    TLibFlexRay AMsg;
} TLibFlexRayHW,*PLibFlexRayHW;

typedef struct _TLibLINHW{
    int HWIdx;
    TLibLIN AMsg;
} TLibLINHW,*PLibLINHW;