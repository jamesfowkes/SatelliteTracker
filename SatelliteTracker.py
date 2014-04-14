import math
import time
import sys
import argparse

import ephem
import serial

from datetime import datetime
from TLEProvider import TLEProvider

SEND_INTERVAL_SECONDS = 2

NOTTM_LONLAT = ('1.13', '52.95')
NOTTM_ELEVATION = 80

class TLEParserError(Exception):
    def __init__(self, expr, msg):
        self.expr = expr
        self.msg = msg
        
class TLEParser:
    
    def __init__(self, observer_latlong, observer_elevation, tle):
        self.setNewTLE(tle)
        self.setNewObserver(observer_latlong, observer_elevation)
        
    def setNewTLE(self, tle):
        try:
            self.tle = tle
            data = tle.getTLE()
            self.sat = ephem.readtle(data[0], data[1], data[2])
        except:
            raise TLEParserError("", "Provided TLE could not be parsed")
            
    def setNewObserver(self, observer_latlong, observer_elevation):
        
        self.observer = ephem.Observer()
        self.observer.lon = observer_latlong[0]
        self.observer.lat = observer_latlong[1]
        self.observer.elevation = observer_elevation
        
    def getAzimuth(self, degrees = True):
        if (degrees):
            return math.degrees(self.sat.az)
        else:
            return self.sat.az
            
    def getAltitude(self, degrees = True):
        if (degrees):
            return math.degrees(self.sat.alt)
        else:
            return self.sat.alt
        
    def update(self, dt = None):
        if dt is None:
            dt = datetime.utcnow()
        self.observer.date = dt
        self.sat.compute(self.observer)

def get_arg_parser():
    """ Return a command line argument parser for this module """
    arg_parser = argparse.ArgumentParser(
        description='TLE Tracker Application for sending azimuth, altitude strings to hardware')
    
    arg_parser.add_argument(
        '--tle', dest='tle', default='ISS', 
        help="Name of the TLE to track")
        
    arg_parser.add_argument(
        '--port', dest='port', default=None,
        help="The serial port to send data to")
        
    arg_parser.add_argument(
        '--baudrate', dest='baudrate', default='9600',
        help="The serial port to send data to")   
        
    return arg_parser
    
def main():
    
    arg_parser = get_arg_parser()
    args = arg_parser.parse_args()
    
    tleProvider = TLEProvider("TLE")
    
    if args.port is not None:
        outputStream = serial.Serial(args.port, int(args.baudrate))
    
    tle = tleProvider.GetTLEByName(args.tle)
    
    tleParser = TLEParser(NOTTM_LONLAT, NOTTM_ELEVATION, tle)

    time.sleep(5)
    outputStream.write("ENGAGE\n")
    time.sleep(5)
    
    while(True):
        try:
            if tleProvider.RefreshTLE(tleParser.tle) == False:
                print("Warning: TLE failed to refresh")
                
            tleParser.update()
            latlongString = "AZ%04dAL%04d\n" % (tleParser.getAzimuth() * 10, tleParser.getAltitude() * 10)
            outputStream.write(latlongString)
            print(latlongString)
            time.sleep(SEND_INTERVAL_SECONDS)
            
        except KeyboardInterrupt:
            outputStream.close()
            exit()
            
        time.sleep(2.0)
        
if __name__ == "__main__":
    main()