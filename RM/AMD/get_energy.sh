#!/bin/bash

while true; do
	sleep 1s
	sensors -u | grep "energy65" >> out/energy/AMD64.txt
done
