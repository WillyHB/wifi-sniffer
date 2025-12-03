#!/bin/bash
sudo iw phy phy0 interface add mon0 type monitor
sudo ip link set mon0 up

