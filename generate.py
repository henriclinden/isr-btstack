#
# Generate.
#
# Henric Linden, rt-labs AB
#

from serial import Serial
from time import time
from time import sleep
from sys import argv


if len(argv) != 2:
	print("usage: {} serialport".format(argv[0]))
	exit(1)

serialport = argv[1]
sp = Serial(serialport, baudrate=57600, rtscts=True, timeout=1.0)
sp.flushInput()
sp.flushOutput()
sleep(0.1)

data = "The quick brown fox jumps over the lazy dog" * 50

for length in [10, 20, 30, 50, 100, 200, 300, 400, 500]:
	for i in range(10):
		req = data[:length - 1] + '\n'
		t1 = time()
		sp.write(req)
		sp.flush()
		d1 = time() - t1
		t2 = time()
		resp = sp.read(length)
		d2 = time() - t2
		if len(resp) == len(req):
			if resp == req:
				print("{0:3d}: {1:0.5f} {2:0.5f} {3:0.5f} {4:0.5f}".format(length, d1, d2, length/d1, length/d2))
			else:
				print("Mismatch. Got {} bytes of {} but content differs.".format(len(resp), len(req)))
				sp.flushInput()
				sp.flushOutput()
				sleep(0.1)
		else:
			print("Mismatch. Got {} bytes of {}".format(len(resp), len(req)))
			sp.flushInput()
			sp.flushOutput()
			sleep(0.1)
	print("")
