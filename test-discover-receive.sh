#!/bin/sh

# Scan for a device, connect and download 10k, repeat.

if [ $# -eq 0 ]
then 
    echo $0 [list of bdaddr] && exit
fi

while true
do
    ./discover | egrep $(echo $* | tr ' ' '|')
    if [ $? -eq 0 ]
    then
        echo $*
        read a b c d <<< "$*" && ./receive $a $b $c $d
    fi
    sleep 1
done
