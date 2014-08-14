from HardwareInterface import HardwareInterface

import serial
import sys

import time

class ArduinoHardware(HardwareInterface):
    
    def __init__(self, port, baudrate):
        HardwareInterface.__init__(self)
        if port is not None:
            self.outstream = serial.Serial(port, int(baudrate))
            self.instream = self.outstream
        else:
            self.outstream = sys.stdout
            self.instream = sys.stdin
        
        self.moving = False
        self.ready = False
        
    def SetPosition(self, az, al):
        print "Sending move to %f, %f" % (az, al)
        latlongString = "AZ%05dAL%05d\n" % (az * 100,  al * 100)
        self.moving = True
        self.outstream.write(latlongString)
        
    @property
    def IsMoving(self):
        return self.moving
        
    @property
    def IsReady(self):
        return self.ready
        
    def Update(self):
        line = self.instream.readline()
        if line.startswith("MOVC"):
            print "Arduino move complete!"
            self.moving = False
        if line.startswith("RDY"):
            print "Arduino ready!"
            self.ready = True
            
    def Start(self):
        self.ready = False
        time.sleep(10)
        self.outstream.write("ENGAGE\n")
        
    def Stop(self):
        self.instream.close()
        self.outstream.close()
