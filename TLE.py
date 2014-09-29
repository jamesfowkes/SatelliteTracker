"""
TLE.py

@author: James Fowkes

Handles a single TLE
"""

import datetime
import time

class TLE:
    """
    Initialises TLE from three lines and the last time it was updated.
    """

    def __init__(self, last_update, id_line, line1, line2):
        self.last_update = last_update
        self.id_line = id_line
        self.line1 = line1
        self.line2 = line2
        self.name = self.id_line[2:]
        self.norad_id = self.line2[2:7]
        self.update_interval_seconds = 2 * 60 * 60 #Default update interval is two hours

    def update_from_other_tle(self, tle):
        """ Change the TLE data """
        if self.id_line == tle.id_line:
            self.line1 = tle.line1
            self.line2 = tle.line2
            self.last_update = datetime.datetime.now()

    @classmethod
    def read(cls, stream_object):
        """ Return a TLE object from reading a stream (probably a text file)
        Assumes that the first line of the file contains the last update time """
        tle_data = stream_object.read().splitlines()

        last_update = datetime.datetime.fromtimestamp(int(tle_data[0]))
        id_line = tle_data[1].decode('utf-8')
        line1 = tle_data[2].decode('utf-8')
        line2 = tle_data[3].decode('utf-8')

        return cls(last_update, id_line, line1, line2)

    def write(self, stream_object):
        """ Writes the current TLE object to stream """
        stream_object.write(str(int(time.time())) + '\n')
        stream_object.write(self.id_line + '\n')
        stream_object.write(self.line1 + '\n')
        stream_object.write(self.line2 + '\n')

    def set_update_interval_seconds(self, new_interval):
        """ Set how often this TLE should be updated from SpaceTrack """
        self.update_interval_seconds = new_interval

    def set_update_interval_minutes(self, new_interval):
        """ Set how often this TLE should be updated from SpaceTrack """
        self.update_interval_seconds = new_interval * 60

    def get_tle(self):
        """ Return the three data lines as an array """
        return [self.id_line, self.line1, self.line2]

    def match_id(self, norad_id):
        """ Match this TLE based on NORAD ID """
        return self.norad_id == norad_id

    def match_name(self, name):
        """ Match this TLE based on name """
        return name in self.name

    def is_old(self):
        """ Return True if the TLE is considered 'old', based on given update interval """
        seconds_since_last_update = (datetime.datetime.now() - self.last_update).total_seconds()
        return seconds_since_last_update > (self.update_interval_seconds)
