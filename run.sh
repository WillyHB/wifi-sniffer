#!/bin/bash

trap cleanup EXIT INT TERM

(cd kern  && ./sniffer_start.sh)
sleep 1
echo opening user program
(cd user && ./user)

cleanup() {
	(cd kern && ./sniffer_stop.sh)
}

cleanup

