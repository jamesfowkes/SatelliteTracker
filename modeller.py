import math
import time
from Position.position import Position
from angle_helper import anglesBridgeZero, RADIANS_PER_TURN

from TLE import TLEProvider
from TLE import TLEParser
from Location.location import Location

SPACING_DEGREES = 2
SPACING_RADIANS = math.radians(SPACING_DEGREES)

class PositionError:

    def __init__(self):
        self.magnitude = 0
        self.leading = False
        self.lagging = True

    def update(self, target, actual):
        self.magnitude = abs(target - actual)

        if anglesBridgeZero(target, actual):
            self.leading = actual > target
        else:
            self.leading = target > actual

        self.lagging = not self.leading

class Model:

    def __init__(self, tle, maxSpeed):
        self.tle = tle
        self.speed = 0
        self.estimatedPosition = Position(0)
        self.targetRelativePosition = Position(0)
        self.lastKnownPosition = Position(0)
        self.targetRelativeSpeed = 0
        self.error = PositionError()
        self.maxSpeed = maxSpeed
        self.fractionalErrorSwitchPoint = 0.05

    def nextPosition(self):
        return self.lastKnownPosition.get() + SPACING_RADIANS

    def distanceToNext(self):
        return abs(self.nextPosition() - self.estimatedPosition.get())

    def tick(self, t):
        self.tle.update()
        self.targetRelativePosition.set( self.tle.getAzimuth(False) )
        self.estimatedPosition.add( -self.speed * t ) ## Speed is +ve going anti-clockwise
        self.error.update(self.targetRelativePosition.get(), self.estimatedPosition.get())

        self.targetRelativeSpeed = self.tle.azimuthSpeed(False)
        self.setNewSpeed()

    def fractionalError(self):
        return self.error.magnitude / RADIANS_PER_TURN

    def useFastMode(self):
        return self.fractionalError() > self.fractionalErrorSwitchPoint

    def setNewSpeed(self):
        if self.useFastMode():
            # Run at fastest possible speed to catch up
            if self.error.leading:
                self.speed = -self.maxSpeed # Ahead by a long way - run in reverse
            else:
                self.speed = +self.maxSpeed
        else:
            if self.error.leading:
                # Reduce speed to let target catch up
                self.speed = self.targetRelativeSpeed * 0.8
            else:
                # Increase speed to catch up
                # Set catch-up speed based on magnitude of error
                speed_diff = (self.maxSpeed - self.targetRelativeSpeed) * self.fractionalError() / self.fractionalErrorSwitchPoint
                self.speed = (self.targetRelativeSpeed + speed_diff)

    def printTitles(self):
        print( "Target RP\tTarget Speed\tEst. P\tError\tSpeed\tLag\Lead" )

    def printVars(self):
        print(
            "{0:.3f}\t\t{1:.3f}\t\t{2:.3f}\t{3:.3f}\t{4:.3f}\t{5}({6})".format(
            self.targetRelativePosition.get(),
            self.targetRelativeSpeed,
            self.estimatedPosition.get(),
            self.error.magnitude,
            self.speed,
            "Leading" if self.error.leading else "Lagging",
            "Fast" if self.useFastMode() else "Track"
            )
        )

    def run(self):
        self.printTitles()
        lastPrintTime = int(time.time())
        self.printVars()
        while(1):
            self.tick(0.001)
            timeNow = int(time.time())
            if (timeNow > lastPrintTime):
            #if 1:
                self.printVars()
                lastPrintTime = timeNow
            time.sleep(0.001)

if __name__ == "__main__":

    tleProvider = TLEProvider.TLEProvider("TLE")
    tle = tleProvider.getTLEByName("ISS (ZARYA)")

    tleParser = TLEParser.TLEParser(
        Location.getLonLat("Nottingham"),
        Location.getElevation("Nottingham"),
        tle)

    model = Model(tleParser, RADIANS_PER_TURN/60)
    model.run()