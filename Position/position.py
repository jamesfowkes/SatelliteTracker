import math
import unittest
from angle_helper import RADIANS_PER_TURN

class Position:

    def __init__(self, startPosition = 0):
        self.set(startPosition)

    def set(self, position):
        self.p = self.normalise(position)

    def add(self, toAdd):
        self.p += toAdd
        self.p = self.normalise(self.p)

    def get(self):
        return self.p

    @staticmethod
    def normalise(position):
        return position % RADIANS_PER_TURN

class PositionTester(unittest.TestCase):

    def testStartPosition(self):
        position = Position(0)
        self.assertEqual(0, position.get())

    def testOverflow(self):
        position = Position(0)
        position.add(-0.1)
        self.assertAlmostEqual(RADIANS_PER_TURN - 0.1, position.get(), 5)

        position.add(0.2)
        self.assertAlmostEqual(0.1, position.get(), 5)

if __name__ == "__main__":

    unittest.main()