


#ifndef PVP_TIMER_H
#define PVP_TIMER_H
#endif

#include <stdint.h>


// Timer Intervals - ALL non-blocking timers
#define LID_OPEN_INTERVAL	120000		// Lid ajar timer interval, sends ajar message if lid is left open
#define LOCKDOWN_INTERVAL	10000		// Period of time before lockdown of vault after lid close, 10 seconds
#define RELAY_INTERVAL		6000		// Lock/Unlock relay operation time for those functions, 6 seconds
#define DEBOUNCE_INTERVAL	200			// Button Debounce
#define RESPONSE_INTERVAL	1500		// Timed response for debug serial.print



