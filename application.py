import math
import sys
import argparse
import logging

from ArduinoHardware import ArduinoHardware

from TLEProvider import TLEProvider
from TLEParser import TLEParser
from Tracker import Tracker

import Location


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
        '--baudrate', dest='baudrate', default='115200',
        help="The serial port to send data to")   

    arg_parser.add_argument(
        '--az', dest='azimuth', default='0',
        help="The starting azimuth of the tracker")   
        
    arg_parser.add_argument(
        '--al', dest='altitude', default='-90',
        help="The starting altitude of the tracker")
    
    arg_parser.add_argument(
        '--loc', dest='location', default='Nottingham',
        help="The location of the tracker on Earth")
        
    return arg_parser
   
def tryToGetTLE(tleName):

    tleProvider = TLEProvider("TLE")
      
    tle = tleProvider.GetTLEByName(tleName)

    if tle is None:
        moduleLogger().info("Could not get TLE by name '%s'. Trying default ID of 25544 (ISS)." % tleName)
        tle = tleProvider.GetTLEByID(25544)
    else:
        moduleLogger().info("Found TLE with name '%s'." % tleName)

    if tle is None:
        moduleLogger().error("No valid TLE could not be found.")
        sys.exit("No valid TLE could not be found.")
        
    return tle, tleProvider

def moduleLogger():
    return logging.getLogger(__name__)
    
def main():

    logging.basicConfig(level=logging.INFO)
    
    moduleLogger().setLevel(logging.INFO)
    
    arg_parser = get_arg_parser()
    args = arg_parser.parse_args()
    
    tle, tleProvider = tryToGetTLE(args.tle)

    location = Location.getLocation(args.location)
    
    tleParser = TLEParser(location, tle)
    motorControl = ArduinoHardware(args.port, args.baudrate, True)
    
    tracker = Tracker(tleProvider, tleParser, motorControl)
    
    tracker.Run(int(args.azimuth), int(args.altitude))
    
if __name__ == "__main__":
    main()
