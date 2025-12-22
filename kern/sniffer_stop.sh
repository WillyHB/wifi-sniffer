#!/bin/bash

echo Unloading Kernel Module
rmmod sniffer.ko

echo Bringing monitor interface down
ip link set "$2" down
iw dev "$2" del

echo Starting NetworkManager and bringing "$1" up
systemctl start NetworkManager
ip link set "$1" up
