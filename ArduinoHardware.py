"""
ArduinoHardware.py

@author: James Fowkes
"""

from IHardware import IHardware

import serial
import sys
import logging

STATE_UNKNOWN = 0
STATE_IDLE = 1
STATE_ONLINE = 2

class ArduinoHardware(IHardware):

    """
    Implements low-level serial control of tracker for Arduino hardware.
    Should implement functions from IHardware
    """

    def __init__(self, port, baudrate, echo):
        IHardware.__init__(self)

        if port is not None:
            self.outstream = serial.Serial(port, int(baudrate))
            self.instream = self.outstream
        else:
            self.outstream = sys.stdout
            self.instream = sys.stdin

        self.state = STATE_UNKNOWN
        self.echo_arduino = echo
        self.update_expected = True ## Expect a message from the Arduino at boot

        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.INFO)

    def set_az_speed(self, dps):
        """ Set the target speed for the azimuth motor """
        self.logger.info("Setting azimuth speed to %f dps", dps)
        string = "AZS%08d\n" % (dps * 100000)
        self.outstream.write(string)

    def set_al_speed(self, dps):
        """ Set the target speed for the altitude motor """
        self.logger.info("Setting altitude speed to %f dps", dps)
        string = "ALS%08d\n" % (dps * 100000)
        self.outstream.write(string)

    def set_target_azimuth(self, az): #pylint: disable=invalid-name
        """ Set the target azimuth """
        self.logger.info("Sending target azimuth %f", az)
        string = "AZ%05d\n" % (az * 100)
        self.outstream.write(string)

    def set_target_altitude(self, al): #pylint: disable=invalid-name
        """ Set the target altitude """
        self.logger.info("Sending target altitude %f", al)
        string = "AL%05d\n" % (al * 100)
        self.outstream.write(string)

    @property
    def is_ready(self):
        """ Return True if the tracker can accept movement commands """
        return self.state != STATE_UNKNOWN

    def update(self):
        """ Read any replies from the Arduino and act on them """
        if self.update_expected:

            ## Assume we got a valid reply
            self.update_expected = False

            line = self.instream.readline()

            if self.echo_arduino:
                self.logger.info("(Got '%s')", line.rstrip())

            if self.state == STATE_UNKNOWN:
                if line.startswith("OFFLINE"):
                    self.logger.info("Arduino ready.")
                    self.state = STATE_IDLE

            elif self.state == STATE_IDLE:
                if line.startswith("ONLINE"):
                    self.logger.info("Motors powered.")
                    self.state = STATE_ONLINE

            elif self.state == STATE_ONLINE:
                if line.startswith("OFFLINE"):
                    self.logger.info("Motors unpowered.")
                    self.state = STATE_IDLE
            else:
                self.logger.info("Unexpected reply '%s' in state %d", line.rstrip(), self.state)
                self.update_expected = True

    def start(self):
        """ Send ENGAGE string to the Arduino to turn on motors """
        self.logger.info("Requesting start")
        self.outstream.write("ENGAGE\n")
        self.update_expected = True

    def stop(self):
        """ Send RELEASE string to the Arduino to turn off motors """
        self.logger.info("Requesting stop")
        self.outstream.write("RELEASE\n")
        self.update_expected = True

    def set_azimuth(self, az): #pylint: disable=invalid-name
        """ Set the current azimuth of the tracker """
        self.outstream.write("AZP%05d\n" % (az * 100))

    def set_altitude(self, az): #pylint: disable=invalid-name
        """ Set the current altitude of the tracker """
        self.outstream.write("ALP%05d\n" % (az * 100))
