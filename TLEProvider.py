
class TLEProvider:
    
    def __init__(self):
        self.tles = [
            [
                "ISS",
                "1 25544U 98067A   14084.18185181  .00024868  00000-0  42670-3 0  9451",
                "2 25544  51.6505 155.9288 0003632 348.6265 105.7694 15.51016925878291"
            ]   
        ]
    
    def GetTLE(self, identifier):
        return next((tle for tle in self.tles if tle[0] == identifier), [None])
        