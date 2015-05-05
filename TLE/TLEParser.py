import math
import time
from datetime import datetime, timedelta

import TLE
import ephem

import unittest

from angle_helper import angleDiff

class Location:

    def __init__(self, az, al):
        self.az = az
        self.al = al

class TLEParserError(Exception):
    def __init__(self, expr, msg):
        self.expr = expr
        self.msg = msg

class TLEParser:

    def __init__(self, observer_latlong, observer_elevation, tle):
        self.setNewTLE(tle)
        self.setNewObserver(observer_latlong, observer_elevation)

        self.oldLocation = Location(0,0)
        self.currentLocation = Location(0,0)
        self.futureLocation = Location(0,0)

    def setNewTLE(self, tle):
        self.tle = tle
        data = tle.getTLE()
        self.sat = ephem.readtle(data[0], data[1], data[2])

    def setNewObserver(self, observer_latlong, observer_elevation):

        self.observer = ephem.Observer()
        self.observer.lon = observer_latlong[0]
        self.observer.lat = observer_latlong[1]
        self.observer.elevation = observer_elevation

    def getAzimuth(self, degrees = True):
        if (degrees):
            return math.degrees(self.currentLocation.az)
        else:
            return self.currentLocation.az

    def getAltitude(self, degrees = True):
        if (degrees):
            return math.degrees(self.currentLocation.al)
        else:
            return self.currentLocation.al

    def azimuthSpeed(self, degrees = True):
        return self.angularSpeed(self.futureLocation.az, self.oldLocation.az)

    def altitudeSpeed(self, degrees = True):
        return self.angularSpeed(self.futureLocation.al, self.oldLocation.al)

    @staticmethod
    def angularSpeed(a, b, degrees = True):
        return angleDiff(a, b, degrees) / 2

    def updateLocation(self, loc, dt):
        self.observer.date = dt
        self.sat.compute(self.observer)
        loc.az = float(self.sat.az)
        loc.al = float(self.sat.alt)

    def update(self, dt = None):

        if dt is None:
            dt = datetime.utcnow()

        self.updateLocation(self.oldLocation, dt - timedelta(seconds = 1))
        self.updateLocation(self.currentLocation, dt)
        self.updateLocation(self.futureLocation, dt + timedelta(seconds = 1))

if __name__ == "__main__":

    unittest.main()