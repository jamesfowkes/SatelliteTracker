#include <Arduino.h>

/* Utility Libraries */
#include "util_angle.h"

/* Application Includes */
#include "MachineIf.h"

void translateMoveRequest(long * pAzimuth, long * pAltitude)
{
#if MACHINE_ID == 1
  /* Shift altitude around 1/4 turn (shift -90 to 90 range into 0-180 range) */
  *pAltitude += 900; 

  /* Correct azimuth and altitude ranges to 0-360 degrees */
  if (*pAzimuth >= 1800)
  {
    // Take reciprocal of azimuth, altitude is mirrored about N-S line.
    *pAzimuth = reciprocal_tdeg(*pAzimuth);
    *pAltitude = mirror_tdeg(*pAltitude, 0);
  }
#elif MACHINE_ID == 2
  // Do nothing: full 360-degree rotation in both planes
#elif MACHINE_ID == 3
  // TODO
#endif

}

