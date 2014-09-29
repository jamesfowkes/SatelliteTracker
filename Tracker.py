"""
Tracker.py

@author: James Fowkes

"""

from task import TaskHandler
from time import sleep

import logging

class Tracker:

    """ Main tracking object.
    Acts as interface between TLE objects and hardware.
    """

    def __init__(self, tle_provider, tle_parser, hardware):
        self.tle_parser = tle_parser
        self.tle_provider = tle_provider
        self.task_handler = TaskHandler(self)
        self.task = self.task_handler.add_function(self.update_task, 100, False)

        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.INFO)

        self.hardware = hardware

    def run(self, start_az, start_al):

        """
        Should be called by application. Does not return.
        Initialises hardware and starts regular updates
        """

        while not self.hardware.is_ready:
            self.hardware.update()

        self.hardware.set_azimuth(start_az)
        self.hardware.set_altitude(start_al)
        self.hardware.start()

        self.task.set_active(True)

        while True:
            sleep(0.01)
            self.task_handler.tick()

    def update_task(self):
        """
        Should be called every second.
        Asks TLE parser to update and sends new data to hardware
        """

        try:
            if self.tle_provider.refresh_tle(self.tle_parser.tle) == False:
                self.logger.warn("Warning: TLE failed to refresh")

            self.tle_parser.update()

            self.hardware.set_az_speed(self.tle_parser.azimuth_speed())
            self.hardware.set_al_speed(self.tle_parser.altitude_speed())
            self.hardware.set_target_azimuth(self.tle_parser.get_azimuth())
            self.hardware.set_target_altitude(self.tle_parser.get_altitude())

            self.hardware.update()

        except KeyboardInterrupt:
            self.hardware.stop()
            exit()
