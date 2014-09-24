"""
application.py

@author: James Fowkes

Entry file for the satellite tracker application
"""

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
        description='TLE Tracker Application')

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

def try_to_get_tle(tle_name):

    """ Attempt to get the TLE for the provided satellite name """
    tle_provider = TLEProvider("TLE")

    tle = tle_provider.GetTLEByName(tle_name)

    if tle is None:
        get_module_logger().info("Could not get TLE by name '%s'. Trying default ID of 25544 (ISS).", tle_name)
        tle = tle_provider.GetTLEByID(25544)
    else:
        get_module_logger().info("Found TLE with name '%s'.", tle_name)

    if tle is None:
        get_module_logger().error("No valid TLE could not be found.")
        sys.exit("No valid TLE could not be found.")

    return tle, tle_provider

def get_module_logger():

    """ Returns logger for this module """
    return logging.getLogger(__name__)

def main():

    """ Application start """

    logging.basicConfig(level=logging.INFO)

    get_module_logger().setLevel(logging.INFO)

    arg_parser = get_arg_parser()
    args = arg_parser.parse_args()

    tle, tle_provider = try_to_get_tle(args.tle)

    location = Location.getLocation(args.location)

    tle_parser = TLEParser(location, tle)
    motor_control = ArduinoHardware(args.port, args.baudrate, True)

    tracker = Tracker(tle_provider, tle_parser, motor_control)

    tracker.Run(int(args.azimuth), int(args.altitude))

if __name__ == "__main__":
    main()
