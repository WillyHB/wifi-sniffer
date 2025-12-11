#!/bin/bash

echo Stopping network manager
systemctl stop NetworkManager
pkill wpa_supplicant

ip link set wlo1 down

echo Creating monitor interface
iw phy phy0 interface add mon0 type monitor #flags all
ip link set mon0 up

echo Loading kernel module
insmod "build/sniffer.ko"
