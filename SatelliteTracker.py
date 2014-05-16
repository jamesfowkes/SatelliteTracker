import math
import sys
import argparse

from ArduinoHardware import ArduinoHardware

from TLEProvider import TLEProvider
from TLEParser import TLEParser

NOTTM_LONLAT = ('1.13', '52.95')
NOTTM_ELEVATION = 80

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
      
    tle = tleProvider.GetTLEByName(args.tle)
    tleParser = TLEParser(NOTTM_LONLAT, NOTTM_ELEVATION, tle)

    motorControl = ArduinoHardware(args.port, args.baudrate)
    motorControl.Start()
    
    while not motorControl.IsReady:
        motorControl.Update()
        
    while(True):
        try:
            if tleProvider.RefreshTLE(tleParser.tle) == False:
                print("Warning: TLE failed to refresh")
                      
            if not motorControl.IsMoving:
                tleParser.update()
                motorControl.SetPosition(tleParser.getAzimuth(), tleParser.getAltitude())
            
            motorControl.Update()
                        
        except KeyboardInterrupt:
            motorControl.Stop()
            exit()

if __name__ == "__main__":
    main()