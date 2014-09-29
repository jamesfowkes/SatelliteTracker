"""
TLEProvider.py

@author: James Fowkes

Maintains a list of TLEs and manages when to update them from SpaceTrack
"""

import os
from SpaceTrack import SpaceTrack
from TLE import TLE

class TLEProvider:
    """ TLEs are pulled in from a single directory """
    def __init__(self, tle_directory):

        self.space_track = SpaceTrack("jamesfowkes@gmail.com", "8rG1eGouVx2aW0X9NzgAj5YDJwaVZO4ciwvmBKi5")

        self.tle_directory = tle_directory
        self.tles = []

        try:
            for file in os.listdir(tle_directory):
                if file.endswith(".tle"):
                    self.add_tle_from_file(tle_directory+"/"+file)
        except FileNotFoundError:
            os.mkdir(tle_directory)

    def add_tle_from_file(self, file):
        """ For a single file, try to read TLE into memory """
        tle_file = open(file, 'rb')
        try:
            tle = TLE.read(tle_file)
            self.tles.append(tle)
        except:
            raise

        tle_file.close()

    def write_tle_to_file(self, tle):
        """ For a single TLE, open a stream for it to write to memory """
        file = open(self.tle_directory + "/" + tle.norad_id + ".tle", 'w', encoding='utf-8')
        tle.write(file)
        file.close()

    def get_local_tle_by_id(self, norad_id):
        """ Search in-memory list for matching NORAD ID """
        return next((tle for tle in self.tles if tle.match_id(norad_id)), None)

    def get_local_tle_by_name(self, name):
        """ Search in-memory list for matching name """
        return next((tle for tle in self.tles if tle.match_name(name)), None)

    def refresh(self, norad_id=None):
        """ Refresh (from SpaceTrack) either a single
        TLE or all of them """
        if norad_id is not None:
            tle = self.get_tle_by_id(norad_id)
            self.refresh_tle(tle)
        else:
            for tle in self.tles:
                self.refresh_tle(tle)

    def get_tle_by_id(self, norad_id):
        """ First search locally, then try download of TLE """
        ## First look in the currently active set of TLEs in RAM
        tle = self.get_local_tle_by_id(norad_id)

        if tle is None:
            # If that didn't work, get it from spacetrack
            tle = self.space_track.get_tle(norad_id)
            if tle is not None:
                self.write_tle_to_file(tle)

        return tle

    def refresh_tle(self, tle):
        """ For provided TLE, update from SpaceTrack if required """
        success = True
        if tle.is_old():
            try:
                new_tle = self.space_track.get_tle(tle.norad_id)
                tle.update_from_other_tle(new_tle)
                self.write_tle_to_file(tle)
            except AttributeError:
                # SpaceTrack refresh failed
                success = False

        return success

