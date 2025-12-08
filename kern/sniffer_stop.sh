#!/bin/bash

echo Unloading Kernel Module
sudo rmmod sniffer.ko

echo Bringing monitor interface down
sudo ip link set mon0 down
sudo iw dev mon0 del

echo Starting NetworkManager and bringing wlo1 up
sudo systemctl start NetworkManager
sudo ip link set wlo1 up
