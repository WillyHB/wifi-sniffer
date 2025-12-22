#!/bin/bash

main_interface=$(ip route show default | awk '{print $5}')

(cd kern  && ./sniffer_start.sh "$main_interface" mon0)
sleep 1
echo opening user program
(cd user && ./user)

cleanup() {
	(cd kern && ./sniffer_stop.sh "$main_interface" mon0)
}

trap cleanup EXIT INT TERM
