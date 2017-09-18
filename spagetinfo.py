#!/usr/bin/env python

from serial import Serial
from spa import SPA

serialports = [
    '/dev/tty.usbserial-A4UV1JUQ',
    '/dev/tty.usbserial-A4XOXZ5L',
    '/dev/tty.usbserial-A4VE6FC1',
    '/dev/tty.usbserial-A4UV1MS2',
    '/dev/tty.usbserial-CBUD3I11',
    '/dev/tty.usbserial-A4UV1KRR',
    '/dev/tty.usbserial-A1002oLm'
]

for port in serialports:
    spa = SPA(Serial(port, baudrate=57600, rtscts=True, timeout=None))
    spa.enterATMode()
    info = spa.command("AT*AILBA?")
    info = info + spa.command("AT*AILVI?")
    print info
    spa.exitATMode()


