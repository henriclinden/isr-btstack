#!/bin/sh

if [ $# -eq 0 ]
then 
    echo $0 [list of bdaddr] && exit
fi

while true
do
    for i in $*
    do
        echo
        echo "--> " [$i]
        ./receive $i
        sleep 1
    done
done
