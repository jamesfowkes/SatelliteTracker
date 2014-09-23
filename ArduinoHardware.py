from HardwareInterface import HardwareInterface

import serial
import sys

import time

import logging

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
        self.updateExpected = True ## Expect a message from the Arduino at boot
        
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.INFO)
        
    def SetAzSpeed(self, dps):
        self.logger.info("Setting azimuth speed to %f dps" % dps)
        string = "AZS%08d\n" % (dps * 100000)
        self.outstream.write(string)
        
    def SetAlSpeed(self, dps):
        self.logger.info("Setting altitude speed to %f dps" % dps)
        string = "ALS%08d\n" % (dps * 100000)
        self.outstream.write(string)
    
    def SetTargetAzimuth(self, az):
        self.logger.info("Sending target azimuth %f" % az)
        string = "AZ%05d\n" % (az * 100)
        self.outstream.write(string)
        
    def SetTargetAltitude(self, al):
        self.logger.info("Sending target altitude %f" % al)
        string = "AL%05d\n" % (al * 100)
        self.outstream.write(string)
               
    @property
    def IsReady(self):
        return self.state != STATE_UNKNOWN
        
    def Update(self):
    
        if self.updateExpected:
        
            ## Assume we got a valid reply
            self.updateExpected = False
            
            line = self.instream.readline()
            
            if self.echoArduino:
                self.logger.info("(Got '%s')" % line.rstrip())
                
            if self.state == STATE_UNKNOWN:
                if line.startswith("OFFLINE"):
                    self.logger.info ("Arduino ready.")
                    self.state = STATE_IDLE
            
            elif self.state == STATE_IDLE:
                if line.startswith("ONLINE"):
                    self.logger.info("Motors powered.")
                    self.state = STATE_ONLINE
            
            elif self.state == STATE_ONLINE:
                if line.startswith("OFFLINE"):
                    self.logger.info("Motors unpowered.")
                    self.state = STATE_IDLE
            else:
                self.logger.info("Unexpected reply '%s' in state %d" % (line.rstrip(), self.state))
                self.updateExpected = True
                
    def Start(self):
        self.logger.info("Requesting start")
        self.outstream.write("ENGAGE\n")
        self.updateExpected = True
        
    def Stop(self):
        self.instream.close()
        self.outstream.close()

    def SetAzimuth(self, az):
        self.outstream.write("AZP%05d\n" % (az * 100))
        
    def SetAltitude(self, az):
        self.outstream.write("ALP%05d\n" % (az * 100))