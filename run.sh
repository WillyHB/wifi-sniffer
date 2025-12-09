#!/bin/bash

echo building kernel
(cd kernel && make)
echo building user program
(cd user && make)

(cd kernel && ./sniffer_start &)
hopper_pid=$!

echo opening user program
(cd user && ./user)

kill "$hopper_pid"
wait "$hopper_pid"

(cd kernel && ./sniffer_stop)
