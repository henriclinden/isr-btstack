#!/bin/sh

while true
do
	./test-reset.sh
	./discover $*
	sleep 0.2
done
