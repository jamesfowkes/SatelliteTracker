import datetime
import time
import os

class TLE:

    def __init__(self, lastUpdate, idLine, line1, line2):
        self.lastUpdate = lastUpdate
        self.idLine = idLine
        self.line1 = line1
        self.line2 = line2
        self.name = self.idLine[2:]
        self.norad_id = self.line2[2:7]
        self.updateIntervalMinutes = 120 #Default update interval is two hours
        
    @classmethod 
    def read(cls, stream_object):
        
        tleData = stream_object.readlines()
        
        lastUpdate = datetime.datetime.fromtimestamp(int(tleData[0]))
        idLine = tleData[1]
        line1 = tleData[2]
        line2 = tleData[3]
        
        return cls(lastUpdate, idLine, line1, line2)
        
    def write(self, stream_object):
        stream_object.write( str(int(time.time())) + os.linesep)
        stream_object.write( self.idLine + os.linesep)
        stream_object.write( self.line1 + os.linesep)
        stream_object.write( self.line2 + os.linesep)

    def setUpdateInterval(self, newInterval):
        self.updateIntervalMinutes = newInterval
        
    def getTLE(self):
        return [self.idLine, self.line1, self.line2]

    def matchName(self, name):
        return name in self.name
        
    def isOld(self):
        isOld = False

        ## Let two hours elapse before requesting again
        try:
            secondsSinceLastUpdate = (datetime.datetime.now() - self.lastUpdate).total_seconds()
            isOld = (secondsSinceLastUpdate > (self.updateIntervalMinutes * 60))
        except TypeError:
            isOld = True #No updates have occured at all, as self.lastUpdate is None
        
        return isOld