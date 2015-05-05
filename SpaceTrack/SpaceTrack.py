import urllib.parse
import urllib.request
import datetime
from TLE import TLE

class SpaceTrack:

    LOGIN_URL = 'https://www.space-track.org/ajaxauth/login'
    QUERY_STRING = 'https://www.space-track.org/basicspacedata/query/class/tle_latest/NORAD_CAT_ID/%s/format/3le/limit/1'

    def __init__(self, identity, pwd):

        self.identity = identity
        self.pwd = pwd
        
    def getTLE(self, norad_cat_id):
    
        postData = urllib.parse.urlencode(
            {'query':self.QUERY_STRING % norad_cat_id, 
            'identity':self.identity,
            'password':self.pwd}
        )    
        
        try:
            tle = urllib.request.urlopen(self.LOGIN_URL, data=postData.encode('ascii')).read().splitlines()
            return TLE(datetime.datetime.now(), tle[0].decode('utf-8'), tle[1].decode('utf-8'), tle[2].decode('utf-8'))
        except:
            raise

def main():
    spaceTrack = SpaceTrack("jamesfowkes@gmail.com", "8rG1eGouVx2aW0X9NzgAj5YDJwaVZO4ciwvmBKi5")
    print (spaceTrack.getTLE("25544"))
    
if __name__ == "__main__":
    main()