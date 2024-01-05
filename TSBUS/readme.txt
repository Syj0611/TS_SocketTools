cd src 目录
Linux:
执行:g++ TSFlexray.cpp -I ../include -L ../lib/linux -lTSCANApiOnLinux -lASCLog -o ../lib/linux/TSFlexray -Wl,-rpath=./:../:/usr/lib
windows:
执行:g++ TSFlexray.cpp -I ../include -L ../lib/linux -llibTSCAN -lws2_32 -o ../lib/windows/TSFlexray.exe

/usr/bin/g++ ./IniMap.cpp -g -I ../include -L ../lib/linux -lTSCANApiOnLinux -lpthread -lASCLog -o ./IniMap -Wl,-rpath=./:../:/usr/lib

v2023.11.10