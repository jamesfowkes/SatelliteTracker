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
            self.tles.append( TLE.read(tleFile) )
        except:
            pass #Malformed TLE file - reject it

        tleFile.close()
        
    def updateTLEList(self, oldTLE, newTLE):
        self.tles[ self.tles.index(oldTLE) ] = newTLE
        
    def writeTLEToFile(self, tle):
        filepath = self.tleDirectory + "/" + tle.norad_id + ".tle"
        file = open (self.tleDirectory + "/" + tle.norad_id + ".tle", 'w')
        tle.write(file)
        file.close()

    def __findTLEByName(self, name):
        return next((tle for tle in self.tles if tle.matchName(name)), None)
        
    def refresh(self, name = None):
        if name is None:
            tle = self.__findTLEByName(name)
            self.RefreshTLE(tle)
            
        else:
            for tle in self.tles:
                self.RefreshTLE(tle)
                
    def GetTLEByName(self, name):
        ## First look in the currently active set of TLEs in RAM
        tle = self.__findTLEByName(name)
        
        if tle is None:
            tle = self.spaceTrack.getTLE(tle.norad_id)

        self.RefreshTLE(tle)

        return tle
        
    def RefreshTLE(self, tle):
        if tle.isOld():
            newTLE = self.spaceTrack.getTLE(tle.norad_id)
            self.updateTLEList(tle, newTLE)
            self.writeTLEToFile( newTLE )
    