import math
import time
from datetime import datetime, timedelta

import TLE
import ephem
import logging

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
        
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.INFO)
        
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
    
    def azimuthSpeed(self):
        
        diff = abs(self.futureLocation.az - self.oldLocation.az)
        
        ## Detect discontinuity in azimuth passing 0 degrees
        ## Since velocity relative to observer is arbitrary for any object, can only rely on big difference between old and new
        ## Fastest relative motion occurs overhead. If it takes an LEO object 3 minutes to transit 180 degrees of sky,
        ## the angular speed is (180 / 4*60) = 0.75 degrees per second.
        ## We are calculating speed over a 2 second window, so maximum change is 3 degrees.
        ## Say any change over 5 degrees is a switch
        
        if diff > 5:
            diff = 360 - diff
            
        return math.degrees(diff) / 2
        
    def altitudeSpeed(self):
        
        diff = abs(self.futureLocation.al - self.oldLocation.al)           
        return math.degrees(diff) / 2
        
    def updateLocation(self, loc, dt):
        self.observer.date = dt
        self.sat.compute(self.observer)
        loc.az = self.sat.az
        loc.al = self.sat.alt

    def update(self, dt = None):
    
        if dt is None:
            dt = datetime.utcnow() 
            
        self.updateLocation(self.oldLocation, dt - timedelta(seconds = 1))
        self.updateLocation(self.currentLocation, dt)
        self.updateLocation(self.futureLocation, dt + timedelta(seconds = 1))
