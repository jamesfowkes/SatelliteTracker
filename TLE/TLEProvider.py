import os
import sys

import time
import datetime
from SpaceTrack import SpaceTrack
from TLE.TLE import TLE

class TLEProvider:

    def __init__(self, tleDirectory):

        self.spaceTrack = SpaceTrack.SpaceTrack("jamesfowkes@gmail.com", "8rG1eGouVx2aW0X9NzgAj5YDJwaVZO4ciwvmBKi5")

        self.tleDirectory = tleDirectory
        self.tles = []

        try:
            for file in os.listdir(tleDirectory):
                if file.endswith(".tle"):
                    self.addTLEFromFile(tleDirectory+"/"+file)
        except FileNotFoundError:
            os.mkdir(tleDirectory)

    def addTLEFromFile(self, file):
        tleFile = open(file, 'rb')
        try:
            tle = TLE.read(tleFile)
            self.tles.append( tle )
        except:
            raise

        tleFile.close()

    def writeTLEToFile(self, tle):
        filepath = self.tleDirectory + "/" + tle.norad_id + ".tle"
        file = open (self.tleDirectory + "/" + tle.norad_id + ".tle", 'w', encoding='utf-8')
        tle.write(file)
        file.close()

    def __findTLEByID(self, id):
        return next((tle for tle in self.tles if tle.matchID(id)), None)

    def __findTLEByName(self, name):
        return next((tle for tle in self.tles if tle.matchName(name)), None)

    def refresh(self, id = None):
        if id is not None:
            tle = self.__findTLEById(id)
            self.refreshTLE(tle)
        else:
            for tle in self.tles:
                self.refreshTLE(tle)

    def getTLEByName(self, name):
        return self.__findTLEByName(name)

    def getTLEByID(self, id):
        ## First look in the currently active set of TLEs in RAM
        tle = self.__findTLEByID(id)

        if tle is None:
            # If that didn't work, get it from spacetrack
            tle = self.spaceTrack.getTLE(id)
            if tle is not None:
                self.writeTLEToFile(tle)

        return tle

    def refreshTLE(self, tle):
        success = True
        if tle.isOld():
            try:
                newTLE = self.spaceTrack.getTLE(tle.norad_id)
                tle.updateFromOtherTLE(newTLE)
                self.writeTLEToFile( tle )
            except AttributeError:
                # SpaceTrack refresh failed
                success = False

        return success

    def listTLEs(self, stream=sys.stdout):
        for tle in self.tles:
            stream.write(tle.name + " " + tle.norad_id + os.linesep)