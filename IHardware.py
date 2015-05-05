"""
IHardware.py

@author: James Fowkes

Interface to be implemented by hardware
"""

class IHardware:
    """
    All pylint warnings are disabled, since class effectively
    does nothing. Should probably be an ABC ->" do this later
    """

    def __init__(self):
        pass

    def set_speed(self, dps): #pylint: disable=missing-docstring
        pass

    def set_target_position(self, az, al): #pylint: disable=invalid-name, missing-docstring
        pass

    def start(self): #pylint: disable=missing-docstring
        pass

    def stop(self): #pylint: disable=missing-docstring
        pass

    def is_moving(self): #pylint: disable=missing-docstring
        pass

    def update(self): #pylint: disable=missing-docstring
        pass

    def set_azimuth(self, az): #pylint: disable=invalid-name, missing-docstring
        pass

    def set_altitude(self, al): #pylint: disable=invalid-name, missing-docstring
        pass
