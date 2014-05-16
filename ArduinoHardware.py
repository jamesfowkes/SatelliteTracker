from HardwareInterface import HardwareInterface

import serial
import sys

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
        latlongString = "AZ%04dAL%04d\n" % (az * 10,  al * 10)
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
        if line == "MOVC\n":
            self.moving = False
        if line == "RDY\n":
            self.ready = True
            
    def Start(self):
        self.ready = False
        self.outstream.write("ENGAGE\n")
        
    def Stop(self):
        self.instream.close()
        self.outstream.close()