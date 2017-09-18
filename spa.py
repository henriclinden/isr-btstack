'''
Created on 14 nov 2010

@author: hl
'''

import re
import time

class SPA:
    STATE_AT_MODE = 1
    STATE_DATA_MODE = 2

    def __init__(self, stream):
        self._stream = stream
        self._state = self.STATE_DATA_MODE
        self._echo = False

    def close(self):
        if (self._state == self.STATE_AT_MODE):
            self.exitATMode()
        return

    def setEchoOn(self, echo):
        self._echo = echo

    def getStream(self):
        return self._stream

    def read(self, size=1):
        return self._stream.read(size)

    def readline(self):
        return self._stream.readline().strip()

    def write(self, data):
        return self._stream.write(data)

    def writeline(self, line):
        self._stream.write(line + '\r')
        self._stream.flush()

    def command(self, command):
        code = re.compile("^(ERROR|OK)$")
        if self._echo:
            print command
        self.writeline(command)
        response = [ ]
        self.readline()
        line = self.readline()
        while (code.match(line) == None):
            response = response + [ line ]
            line = self.readline()
        if self._echo:
            print line
        return response

    def enterATMode(self, esc='///', timeout=1.1):
        if (self._state == self.STATE_DATA_MODE):
            time.sleep(timeout)
            self._stream.write(esc)
            time.sleep(timeout)
            self._state = self.STATE_AT_MODE
            self.command('AT')

    def exitATMode(self):
        if (self._state == self.STATE_AT_MODE):
            # self.writeline("AT*ADDM")
            # Bluetooth modules return OK before entering data mode. Wireless LAN does not.
            self.command("AT*ADDM")
            self._state = self.STATE_DATA_MODE

    def exitATModeAndReset(self):
        if (self._state == self.STATE_AT_MODE):
            self.writeline("AT*AMWS=0,0,0,0,1,0")
            self._state = self.STATE_DATA_MODE
            time.sleep(0.2)

