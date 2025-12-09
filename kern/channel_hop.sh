#!/bin/bash

echo Starting Channel Hopping
while true; do
	for c in 1 6 11 36 40 44 48; do
		iw dev mon0 set channel $c
		sleep 0.1
	done
done
