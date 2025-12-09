#!/bin/bash

echo Stopping network manager
sudo systemctl stop NetworkManager
sudo pkill wpa_supplicant

sudo ip link set wlo1 down

echo Creating monitor interface
sudo iw phy phy0 interface add mon0 type monitor
sudo ip link set mon0 up

echo Loading kernel module
sudo insmod "build/sniffer.ko"

echo Starting Channel Hopping
while true; do
	for c in 1 6 11 36 40 44 48; do
		sudo iw dev mon0 set channel $c
		sleep 0.1
	done
done
