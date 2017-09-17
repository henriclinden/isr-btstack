#!/bin/sh

# Scan for a device, connect and download 10k, repeat.

while true
do
    ./discover | grep 78:C5:E5:A1:E5:51
    if [ $? -eq 0 ]
    then
        ./receive 78:C5:E5:A1:E5:51
    fi
    sleep 1
done
