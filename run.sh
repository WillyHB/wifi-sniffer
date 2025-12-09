#!/bin/bash


echo make kernel module
(cd kern && make)
echo make user program
(cd user && make)

cd kern  || exit 1
./sniffer_start.sh

echo starting channel hopper
./channel_hop.sh &
hopper_pid=$!
cd ..

echo opening user program
(cd user && ./user)

cleanup() {
	kill "$hopper_pid"
	wait "$hopper_pid"

	(cd kern && ./sniffer_stop.sh)
}

trap cleanup EXIT INT TERM
