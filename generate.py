#
# Generate.
#
# Henric Linden, rt-labs AB
#

from serial import Serial
from time import time
from time import sleep
from sys import argv
from sys import stdout

def lprint(string):
    stdout.write(string)
    stdout.flush()

if __name__ == "__main__":
    serialport = argv[1]
    sp = Serial(serialport, baudrate=57600, rtscts=True, timeout=1.0)

    data = "The quick brown fox jumps over the lazy dog" * 50

    for length in [10, 20, 30, 50, 100, 200, 300, 400, 500]:
        print "Length", length, "x10"
        for i in range(10):
            lprint("<")
            req = data[:length - 1] + '\n'
            sp.write(req)
            sp.flush()
            resp = sp.read(length)
            if len(resp) == len(req) and resp == req:
                lprint(">")
            else:
                lprint("!")
        print ""
