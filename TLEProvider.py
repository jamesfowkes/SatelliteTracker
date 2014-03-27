import os
import time
import datetime
from SpaceTrack import SpaceTrack
from TLE import TLE

class TLEProvider:
            
    def __init__(self, tleDirectory):
        
        self.spaceTrack = SpaceTrack("jamesfowkes@gmail.com", "8rG1eGouVx2aW0X9NzgAj5YDJwaVZO4ciwvmBKi5")
        
        self.tleDirectory = tleDirectory
        self.tles = []
        for file in os.listdir(tleDirectory):
            if file.endswith(".tle"):
                self.addTLEFromFile(tleDirectory+"/"+file)

    def addTLEFromFile(self, file):
        tleFile = open(file, 'r')
        try:
            tle = TLE.read(tleFile)
            self.tles.append( tle )
        except:
            raise

        tleFile.close()
        
    def writeTLEToFile(self, tle):
        filepath = self.tleDirectory + "/" + tle.norad_id + ".tle"
        file = open (self.tleDirectory + "/" + tle.norad_id + ".tle", 'w')
        tle.write(file)
        file.close()

    def __findTLEById(self, id):
        return next((tle for tle in self.tles if tle.matchID(id)), None)
        
    def __findTLEByName(self, name):
        return next((tle for tle in self.tles if tle.matchName(name)), None)
        
    def refresh(self, id = None):
        if id is not None:
            tle = self.__findTLEById(id)
            self.RefreshTLE(tle)
        else:
            for tle in self.tles:
                self.RefreshTLE(tle)
                
    def GetTLEByName(self, name):
        return self.__findTLEByName(name)
        
    def GetTLEByID(self, id):
        ## First look in the currently active set of TLEs in RAM
        tle = self.__findTLEByID(id)
        
        if tle is None:
            tle = self.spaceTrack.getTLE(id)
            try:
                self.RefreshTLE(tle)
            except TypeError:
                return None

        return tle
        
    def RefreshTLE(self, tle):
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
            