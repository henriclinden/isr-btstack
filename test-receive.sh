#!/bin/sh

if [ $# -eq 0 ]
then
    echo $0 [list of bdaddr] && exit
fi

while true
do
	./test-reset.sh
	./receive $*
	sleep 0.2
done
