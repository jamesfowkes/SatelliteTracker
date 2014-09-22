from HardwareInterface import HardwareInterface

import serial
import sys

import time

STATE_UNKNOWN = 0
STATE_IDLE = 1
STATE_ONLINE = 2

class ArduinoHardware(HardwareInterface):
    
    def __init__(self, port, baudrate, echo):
        HardwareInterface.__init__(self)
        if port is not None:
            self.outstream = serial.Serial(port, int(baudrate))
            self.instream = self.outstream
        else:
            self.outstream = sys.stdout
            self.instream = sys.stdin
        
        self.state = STATE_UNKNOWN
        self.echoArduino = echo
    
    def SetAzSpeed(self, dps):
        print("Setting azimuth speed to %f dps" % dps)
        string = "AZS%05d\n" % (dps * 100)
        self.outstream.write(string)
        
    def SetAlSpeed(self, dps):
        print("Setting altitude speed to %f dps" % dps)
        string = "ALS%05d\n" % (dps * 100)
        self.outstream.write(string)
    
    def SetTargetAzimuth(self, az):
        print("Sending target azimuth %f" % az)
        string = "AZ%05d\n" % (az * 100)
        self.outstream.write(string)
        
    def SetTargetAltitude(self, al):
        print("Sending target altitude %f" % al)
        string = "AL%05d\n" % (al * 100)
        self.outstream.write(string)
               
    @property
    def IsReady(self):
        return self.state != STATE_UNKNOWN
        
    def Update(self):
        line = self.instream.readline()
        
        if self.echoArduino:
            print("(Got '%s')" % line.rstrip())
            
        if self.state == STATE_UNKNOWN:
            if line.startswith("OFFLINE"):
                print ("Arduino ready.")
                self.state = STATE_IDLE
        
        if self.state == STATE_IDLE:
            if line.startswith("ONLINE"):
                print("Motors powered.")
                self.state = STATE_ONLINE
        
        if self.state == STATE_ONLINE:
            if line.startswith("OFFLINE"):
                print("Motors unpowered.")
                self.state = STATE_IDLE
            
    def Start(self):
        self.outstream.write("ENGAGE\n")
        
    def Stop(self):
        self.instream.close()
        self.outstream.close()

    def SetAzimuth(self, az):
        self.outstream.write("AZP%05d\n" % (az * 100))
        
    def SetAltitude(self, az):
        self.outstream.write("ALP%05d\n" % (az * 100))