#!/bin/bash

echo Unloading Kernel Module
rmmod sniffer.ko

echo Bringing monitor interface down
ip link set mon0 down
iw dev mon0 del

echo Starting NetworkManager and bringing wlo1 up
systemctl start NetworkManager
ip link set wlo1 up
