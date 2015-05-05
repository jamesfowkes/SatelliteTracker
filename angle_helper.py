import math
import unittest

RADIANS_PER_TURN = 2 * math.pi

def normalise(a, degrees = True):
    if degrees:
        return a % 360
    else:
        return a % (2 * math.pi)

def anglesBridgeZero(a, b, degrees = False):

        if a == b:
            return False

        halfRevolution = RADIANS_PER_TURN / 2
        if degrees:
            halfRevolution = 180

        lowest = min(a, b)
        highest = max(a, b)

        diff = highest - lowest

        crosses = True

        if diff > halfRevolution:
            # If the absolute difference between high and low is > 180degrees,
            # then the angle can only cross zero if the lowest is less than 180degrees
            crosses &= lowest < halfRevolution
        else:
            # If the absolute difference between high and low is < 180degrees,
            # then the angle can only cross zero if the lowest is greater than 180degrees
            # and the highest is less than 360degrees
            crosses &= lowest > halfRevolution
            crosses &= highest < halfRevolution

        return crosses

def angleDiff(a, b, degrees = True):

    a = normalise(a)
    b = normalise(b)

    diff = abs(a - b)

    if degrees:
        if diff > 180:
            diff = 360 - diff
    else:
        if diff > math.pi:
            diff = (2 * math.pi) - diff

    return diff

class AngleTester(unittest.TestCase):

    def testSameAnglesReturnFalse(self):
        self.assertFalse(anglesBridgeZero(0,0, True))
        self.assertFalse(anglesBridgeZero(1,1, True))
        self.assertFalse(anglesBridgeZero(359,359, True))

    def testAnglesBetweenZeroAndAnotherAngle(self):
        self.assertFalse(anglesBridgeZero(0,1, True))
        self.assertFalse(anglesBridgeZero(0,179, True))
        self.assertFalse(anglesBridgeZero(0,180, True))

        self.assertTrue(anglesBridgeZero(0,181, True))
        self.assertTrue(anglesBridgeZero(0,359, True))

    def testAngleThatBridgeZeroReturnTrue(self):
        self.assertTrue(anglesBridgeZero(1, 359, True))
        self.assertTrue(anglesBridgeZero(1, 182, True))

    def testAnglesThatDoNotBridgeZeroReturnFalse(self):
        self.assertFalse(anglesBridgeZero(1, 180, True))
        self.assertFalse(anglesBridgeZero(1, 181, True))

    def testDiffReturnsCorrectValues(self):

        self.assertEqual(0, angleDiff(0,0))
        self.assertEqual(0, angleDiff(0,360))

        self.assertEqual(1, angleDiff(0,1))

        self.assertEqual(1, angleDiff(359,0))
        self.assertEqual(2, angleDiff(359,1))

        self.assertEqual(180, angleDiff(0,180))
        self.assertEqual(180, angleDiff(90,270))

        self.assertAlmostEqual(math.radians(0), angleDiff(math.radians(0),math.radians(0), False))
        self.assertAlmostEqual(math.radians(0), angleDiff(math.radians(0),math.radians(360), False))

        self.assertAlmostEqual(math.radians(1), angleDiff(math.radians(0),math.radians(1), False))

        self.assertAlmostEqual(math.radians(1), angleDiff(math.radians(359),math.radians(0), False))
        self.assertAlmostEqual(math.radians(2), angleDiff(math.radians(359),math.radians(1), False))

        self.assertAlmostEqual(math.radians(180), angleDiff(math.radians(0),math.radians(180), False))
        self.assertAlmostEqual(math.radians(180), angleDiff(math.radians(90),math.radians(270), False))

if __name__ == "__main__":

    unittest.main()
