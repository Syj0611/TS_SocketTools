#! /bin/sh
ip link set can0 down
ip link set can1 down
ip link set can2 down
ip link set can3 down
ip link set can4 down
ip link set can5 down

ip link set can0 up type can bitrate 500000 sample-point 0.75 dsample-point 0.80 dbitrate 2000000 fd on restart-ms 300
ip link set can1 up type can bitrate 500000 sample-point 0.75 dsample-point 0.80 dbitrate 2000000 fd on restart-ms 300
ip link set can2 up type can bitrate 500000 sample-point 0.75 dsample-point 0.80 dbitrate 2000000 fd on restart-ms 300
ip link set can3 up type can bitrate 500000 sample-point 0.75 dsample-point 0.80 dbitrate 2000000 fd on restart-ms 300

ip link set can4 up type can bitrate 500000 restart-ms 300
ip link set can5 up type can bitrate 500000 restart-ms 300


