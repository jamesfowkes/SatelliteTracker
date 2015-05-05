import math
import time
from datetime import datetime
import ephem

degrees_per_radian = 180.0 / math.pi

home = ephem.Observer()
home.lon = '1.13'
home.lat = '52.95'
home.elevation = 80 # meters

iss = ephem.readtle('ISS',
    '1 25544U 98067A   15124.51397071  .00016717  00000-0  10270-3 0  9003',
    '2 25544  51.6418 302.1562 0005592 298.4190  61.6400 15.56352367 21249'
)

while True:
    home.date = datetime.utcnow()
    iss.compute(home)
    print('iss: altitude %0.4f rads, azimuth %0.4f rads' % (iss.alt, iss.az))
    time.sleep(1.0)