import math
import time
from datetime import datetime

import TLE
import ephem

class TLEParserError(Exception):
    def __init__(self, expr, msg):
        self.expr = expr
        self.msg = msg
        
class TLEParser:
    
    def __init__(self, observer_latlong, observer_elevation, tle):
        self.setNewTLE(tle)
        self.setNewObserver(observer_latlong, observer_elevation)
        
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