"""
TLEParser.py

@author: James Fowkes
"""

import math
from datetime import datetime, timedelta

import ephem
import logging

import unittest

from angle_helper import angleDiff

class Location:
    """ Small class to store the relative location of an object """
     #pylint: disable=too-few-public-methods
    def __init__(self, az, al):  #pylint: disable=invalid-name
        self.az = az #pylint: disable=invalid-name
        self.al = al #pylint: disable=invalid-name

class TLEParserError(Exception):
    """ Exception class to raise on an TLE parse error """
    #pylint: disable=super-init-not-called
    def __init__(self, expr, msg):
        self.expr = expr
        self.msg = msg

class TLEParser:
    """ Main parser class. Is passed a TLE object and an
    observer and provides Locations for the TLE object """
    def __init__(self, observer, tle):
        self.sat = None
        self.tle = None
        self.observer = ephem.Observer()
        self.set_new_tle(tle)
        self.set_new_observer(observer)

        self.old_location = Location(0, 0)
        self.current_location = Location(0, 0)
        self.future_location = Location(0, 0)

        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.INFO)

    def set_new_tle(self, tle):
        """ Changes TLE and resets the sat member from the ephem module """
        self.tle = tle
        data = tle.get_tle()
        self.sat = ephem.readtle(data[0], data[1], data[2])

    def set_new_observer(self, observer):
        """ Changes the observer """
        self.observer = ephem.Observer()
        self.observer.lat = observer.lat
        self.observer.lon = observer.lon
        self.observer.elevation = observer.elevation

    def get_azimuth(self, degrees=True):
        """ Returns the current azimuth of the tracked object """
        if degrees:
            return math.degrees(self.current_location.az)
        else:
            return self.current_location.az

    def get_altitude(self, degrees=True):
        """ Returns the current altitude of the tracked object """
        if degrees:
            return math.degrees(self.current_location.al)
        else:
            return self.current_location.al

    def azimuth_speed(self, degrees = True):
        """ Returns the speed of the azimuth across the sky (in degrees per second) """
        return self.angularSpeed(self.future_location.az, self.old_location.az)

    def altitude_speed(self, degrees = True):
        """ Returns the speed of the altitude across the sky (in degrees per second) """
        return self.angularSpeed(self.future_location.al, self.old_location.al)

    @staticmethod
    def angularSpeed(a, b, degrees = True):
        return angleDiff(a, b, degrees) / 2

    def update_location(self, loc, dt): #pylint: disable=invalid-name
        """ Calls the compute method on the currently tracked object
        using the provided datetime """
        self.observer.date = dt
        self.sat.compute(self.observer)
        loc.az = float(self.sat.az)
        loc.al = float(self.sat.alt)

    def update(self, dt=None): #pylint: disable=invalid-name

        """ Recalculates position and speed for the provided datetime """
        if dt is None:
            dt = datetime.utcnow()

        self.update_location(self.old_location, dt - timedelta(seconds=1))
        self.update_location(self.current_location, dt)
        self.update_location(self.future_location, dt + timedelta(seconds=1))

if __name__ == "__main__":

    unittest.main()