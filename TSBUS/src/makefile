all:IniMap,test_init

IniMap:IniMap.cpp
	/usr/bin/g++ /home/jx/Desktop/new_so_TSBUS/TSBUS_SocketTools/TSBUS/src/IniMap.cpp -g -O2 -I ../include -L ../lib/linux -lTSCANApiOnLinux -lpthread -lASCLog -o /home/jx/Desktop/new_so_TSBUS/TSBUS_SocketTools/TSBUS/lib/linux/IniMap -Wl,-rpath=./:../:/usr/lib
test_init:test_init.cpp
	/usr/bin/g++ ./test_init.cpp -g -I ../include -L ../lib/linux -lTSCANApiOnLinux -o ./test_init 


