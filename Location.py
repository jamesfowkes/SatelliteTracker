"""
Location.py

@author: James Fowkes

Provides location information for points on Earth
"""

#There's only one public method, this is OK.
#pylint: disable=too-few-public-methods

import logging

class Location:

    """ Simple class to store lat, long and alt """

    def __init__(self, lat, long, alt):

        self.lat = lat
        self.long = long
        self.alt = alt

LOCATIONS = {
    "Nottingham" : Location('1.13', '52.95', 80),
    "Bletchley" : Location('0.767', '51.97', 80)
}

def get_location(index):

    """
    Return the Location object for the requested location,
    or for Nottingham if that location can't be found
    """

    logger = logging.getLogger(__name__)

    location = None

    try:
        logger.info("Found location '%s'", index)
        location = LOCATIONS[index]
    except KeyError:
        logger.warn("Location '%s' not found, defaulting to Nottingham", index)
        location = LOCATIONS["Nottingham"]

    return location
