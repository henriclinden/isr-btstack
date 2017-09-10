#
# Flood serial channels with data.
#
# Henric Linden, rt-labs AB
#

from serial import Serial
from threading import Thread
from threading import Event
from time import time
from time import sleep

serialports = [
    '/dev/tty.usbserial-A4UV1JUQ',
    '/dev/tty.usbserial-A4XOXZ5L',
    '/dev/tty.usbserial-A4VE6FC1',
    '/dev/tty.usbserial-A4UV1MS2',
    '/dev/tty.usbserial-CBUD3I11',
    '/dev/tty.usbserial-A4UV1KRR',
    '/dev/tty.usbserial-A1002oLm'
]

class FloodThread(Thread):
    def __init__(self, stream, id, stats):
        Thread.__init__(self)
        self.stream = stream
        self.id = id
        self.is_stopped = Event()
        self.stats = stats
        self.n = 0

    def run(self):
        while not self.is_stopped.is_set():
            data = "{0} {1:.2f} The quick brown fox jumps over the lazy dog\n".format(self.id, time())
            self.n = self.n + len(data)
            self.stream.write(data)

    def stop(self):
        self.is_stopped.set()
        self.stats[self.id] = self.n


threads = []
stats = {}

if __name__ == "__main__":
    try:
        print "Creating threads."
        for sp in serialports:
            # import sys
            # ss = sys.stdout
            ss = Serial(sp, baudrate=57600, rtscts=True, timeout=None)
            threads.append(FloodThread(ss, sp.split('-', 1)[1], stats))

        start_time = time()
        print "Starting threads."
        for th in threads:
            th.daemon = True
            th.start()

        print "Running. Press Ctrl-C to stop."
        while True:
            sleep(1)

    except KeyboardInterrupt:
        print ""
        print "Stop."
        for th in threads:
            th.stop()
            th.join()

    stop_time = time()
    duration_time = stop_time - start_time

    print "Duration: {0:.2f} seconds.".format(duration_time)
    print "Number of bytes sent per interface:"
    for key in stats:
        print " {0} = {1} bytes, {2:.2f} bytes/sec".format(key, stats[key], stats[key] / duration_time)
