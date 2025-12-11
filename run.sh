#!/bin/bash


echo make kernel module
(cd kern && make) || exit 1
echo make user program
(cd user && make) || exit 1

cd kern  || exit 1
./sniffer_start.sh


#echo starting channel hopper
#./channel_hop.sh & 
#hopper_pid=$!

cd ..

sleep 1
echo opening user program
(cd user && ./user)

cleanup() {
	#kill "$hopper_pid"
	#wait "$hopper_pid"

	(cd kern && ./sniffer_stop.sh)
}

trap cleanup EXIT INT TERM
