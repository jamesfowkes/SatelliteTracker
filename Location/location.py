LON_LATS = {
    "Nottingham" : ('1.13', '52.95'),
    "Bletchley" : ('0.767', '51.97')
}

ELEVATIONS = {
    "Nottingham" : 80,
    "Bletchley" : 80
}

class Location:

    @staticmethod
    def getLonLat(location):
        return LON_LATS[location]

    @staticmethod
    def getElevation(location):
        return ELEVATIONS[location]
