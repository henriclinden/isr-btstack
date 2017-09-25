#!/bin/sh

echo "Reset WL1273 @ gpio83"
echo "0" > /sys/class/gpio/gpio83/value
sleep 0.1
echo "1" > /sys/class/gpio/gpio83/value

