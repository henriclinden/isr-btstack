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

    spa.setEchoOn(True)
    spa.enterATMode()

    # Reset to factory defaults.
    spa.command("AT&F")

    # Get model and type.
    major_minor = spa.command("AT*AILTI?")[0][7:]
    if major_minor == "1,11":
        # A dual-mode module. Enable BLE periferal mode + serial port service.
        print "Module is an cB-OBS421 with dual-mode Bluetooth. Enable BLE."
        spa.command("AT*AGLE=2,1")
        spa.command("AT*ADDSP=14,1")
    elif major_minor == "1,12":
        # Its an single-mode module. OLP425/OLS425/6.
        print "Module is an cB-OLS425/6."
    else:
        print "Module is an not an Bluetooth module. Skip."
        continue

    # Allow only online mode. The Serial Port Adapter will never enter sleep or stop mode to save power.
    spa.command("AT*AMPM=1,1")

    # dsr_cfg=0b101100: DSR does not affect stop mode.
    #                   When DSR is active, discoverability and connectability are enabled (advertising).
    #                   External connect is not affected.
    #                   A DSR transition from active to inactive will trigger a disconnect.
    spa.command("AT*AMDS2=0,44,1")

    # Set escape sequence timing.
    # spa.command("AT*AMET=50,50,1")

    # Configuration over Bluetooth is allowed.
    # spa.command("AT*ACCB=1,1")

    # Set default name.
    spa.command("AT*AGLN=\"DeLaval ISR Sensor\",1")

    # Setup fine grained control of connection interval.
    for command, arg in [
        ["ATS6000",     "200"],     # link supervision timeout to 200ms
        ["ATS6004",     "8"  ],     # Connection Interval Min = 8, where 8 corresponds to 10ms (8*1.25ms=10)
        ["ATS6005",     "8"  ],     # Connection Interval Max = 8, , where 8 corresponds to 10ms (8*1.25ms=10)
        ["ATS6006",     "0"  ],     # Slave Latency = 0
        ["ATS6011",     "6"  ],     # trigger a Connection_Update_Request  if Connection Interval  < 7.5ms (6*1.25ms=7.5ms)
        ["ATS6012",     "8"  ],     # trigger a Connection_Update_Request  if Connection Interval  > 10ms (8*1.25ms=10ms)
        ["ATS6013",     "0"  ],     # trigger a Connection_Update_Request  if Slave latency < 0
        ["ATS6014",     "0"  ],     # trigger a Connection_Update_Request  if Slave latency > 0
        ["ATS6015",     "100"],     # trigger a Connection_Update_Request  if link timeout < 100ms. [>=6 x connection interval]
        ["ATS6016",     "300"],     # trigger a Connection_Update_Request  if link timeout > 300ms. [>=6 x connection interval]
    ]:
        spa.command(command + "=" + arg)
    
    # All done.
    spa.exitATModeAndReset()




