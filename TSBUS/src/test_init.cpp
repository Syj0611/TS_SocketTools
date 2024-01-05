#include <string>
#include <cstring>
#include "TSCANDef.hpp"
#include <iostream>
using namespace std;

s32 aa = 1;

int main()
{
    initialize_lib_tscan(true, false, false);

    uint32_t ADeviceCount;
    tscan_scan_devices(&ADeviceCount);
    cout << "Online device num: " << ADeviceCount << endl;

    char *AFManufacturer;
    char *AFProduct;
    char *AFSerial;

    for (uint32_t i = 0; i < ADeviceCount; i++)
    {
        tscan_get_device_info(i, &AFManufacturer, &AFProduct, &AFSerial);
        cout << "Manufacturer:" << AFManufacturer << endl;
        cout << "Product No:" << AFProduct << endl;
        cout << "Serial No:" << AFSerial << endl;
    }
    cout << &aa << endl;
    // tscan_config_canfd_by_baudrate(0,(APP_CHANNEL)CHN1,500,2000,lfdtISOCAN,lfdmNormal,1);
    // TLibCANFD AMsg;
    // AMsg.FIdentifier = 0x127;
    // AMsg.FDLC = 8;
    // u8 isfd = 0;
    // u8 isbrs = 0;
    // u8 isstd = 0;
    // u16 Cycletime = 0.01;
    // AMsg.SetIsFD((isfd==1));
    // AMsg.SetStd(isstd==1);
    // AMsg.SetIsBRS(isbrs ==1);
    // tscan_add_cyclic_msg_canfd(0,&AMsg,Cycletime*1000);

    // tscan_config_canfd_by_baudrate(0,(APP_CHANNEL)CHN1,500,2000,lfdtISOCAN,lfdmNormal,1);
    // AMsg.FIdentifier = 0x401;
    // AMsg.FDLC = 8;
    // isfd = 0;
    // isbrs = 0;
    // isstd = 0;
    // Cycletime = 0.01;
    // AMsg.SetIsFD((isfd==1));
    // AMsg.SetStd(isstd==1);
    // AMsg.SetIsBRS(isbrs ==1);
    // tscan_add_cyclic_msg_canfd(0,&AMsg,Cycletime*1000);
}
