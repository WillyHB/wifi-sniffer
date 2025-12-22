#!/bin/bash

echo Stopping network manager
systemctl stop NetworkManager
pkill wpa_supplicant

ip link set "$1" down

echo Creating monitor interface
iw phy phy0 interface add "$2" type monitor #flags all
ip link set "$2" up

echo Loading kernel module
insmod "build/sniffer.ko"
