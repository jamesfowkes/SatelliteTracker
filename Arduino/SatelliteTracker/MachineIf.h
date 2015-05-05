#ifndef _MACHINE_IF_H_
#define _MACHINE_IF_H_

/*
 * Defines and Typedefs
 */
 
/*
 * Machine IDs
 * 1 - Original prototype
 * 2 - Massive tracker
 * 3 - Small tracker
 */
 
#define MACHINE_ID 2

/*
 * Public Function Prototypes
 */
 
void translateMoveRequest(long * pAzimuth, long * pAltitude);

#endif
