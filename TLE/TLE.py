import datetime
import time
import os

class TLE:

    def __init__(self, lastUpdate, idLine, line1, line2):
        self.lastUpdate = lastUpdate
        self.idLine = idLine
        self.line1 = line1
        self.line2 = line2
        self.name = self.idLine
        self.norad_id = self.line2[2:7]
        self.updateIntervalSeconds = 2 * 60 * 60 #Default update interval is two hours

    def updateFromOtherTLE(self, tle):
        if self.idLine == tle.idLine:
            self.line1 = tle.line1
            self.line2 = tle.line2
            self.lastUpdate = datetime.datetime.now()

    @classmethod
    def read(cls, stream_object):

        tleData = stream_object.read().splitlines()

        lastUpdate = datetime.datetime.fromtimestamp(int(tleData[0]))
        idLine = tleData[1].decode('utf-8')
        line1 = tleData[2].decode('utf-8')
        line2 = tleData[3].decode('utf-8')

        return cls(lastUpdate, idLine, line1, line2)

    def write(self, stream_object):
        stream_object.write( str(int(time.time())) + '\n')
        stream_object.write( self.idLine + '\n')
        stream_object.write( self.line1 + '\n')
        stream_object.write( self.line2 + '\n')

    def setUpdateIntervalSeconds(self, newInterval):
        self.updateIntervalSeconds = newInterval

    def setUpdateIntervalMinutes(self, newInterval):
        self.updateIntervalSeconds = newInterval * 60

    def getTLE(self):
        return [self.idLine, self.line1, self.line2]

    def matchID(self, id):
        return self.norad_id == id

    def matchName(self, name):
        return name in self.name

    def isOld(self):
        secondsSinceLastUpdate = (datetime.datetime.now() - self.lastUpdate).total_seconds()
        return (secondsSinceLastUpdate > (self.updateIntervalSeconds ))
