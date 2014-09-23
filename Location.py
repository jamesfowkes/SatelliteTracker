import logging

class Location:

    def __init__(self, lat, long, alt):
    
        self.lat = lat
        self.long = long
        self.alt = alt

locations = {
    "Nottingham" : Location('1.13', '52.95', 80),
    "Bletchley" : Location('0.767', '51.97', 80)
}

def getLocation(index):

    logger = logging.getLogger(__name__)
    
    location = None
    
    try:
        logger.info("Found location '%s'" % index)
        location = locations[index]
    except:
        logger.warn("Location '%s' not found, defaulting to Nottingham" % index)
        location = locations["Nottingham"]

    return location