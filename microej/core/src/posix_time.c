/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include "posix_time.h"
#include <time.h>
#include <nuttx/clock.h>

#ifdef __cplusplus
	extern "C" {
#endif

#include "microej.h"

/*
 *********************************************************************************************************
 *                                             DEFINES
 *********************************************************************************************************
 */

#define NANO_TO_MILLIS		1000000
#define MILLIS_TO_SECONDS	1000

/*
 *********************************************************************************************************
 * 	                                      INTERNAL FUNCTIONS
 *********************************************************************************************************
 */

volatile int64_t system_time_offset = 0;
/*
 *********************************************************************************************************
 * 	                                      PUBLIC FUNCTIONS
 *********************************************************************************************************
 */
void posix_time_initialize_system_time_offset()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	int64_t milliseconds = ((ts.tv_sec) * MILLIS_TO_SECONDS);
	milliseconds += ((ts.tv_nsec) / NANO_TO_MILLIS);

	system_time_offset = milliseconds - posix_time_getcurrenttime(1);
}

int64_t posix_time_getcurrenttime(uint8_t isPlatformTime){
	// /!\
	// isPlatformTime == true when ej.bon.Util.platformTimeMillis
	// isPlatformTime == false when java.lang.System.currentTimeMillis
	// Posix MONOTONIC is equivalent to B-ON Platform time
	// Posix REALTIME is equivalent to B-ON Application time and Java System time
	// /!\

	struct timespec ts;
	if(!isPlatformTime)
	{
		return system_time_offset + MSEC_PER_TICK * clock_systimer();
	}
	else
	{
		return MSEC_PER_TICK * clock_systimer();
	}

}

int64_t posix_time_gettimenanos(){
	return NSEC_PER_TICK * clock_systimer();
}

void posix_time_setapplicationtime(int64_t t)
{
	system_time_offset = (t - posix_time_getcurrenttime(1));
}

/**	Compute absolute realtime from absolute monotonic time */
int64_t posix_time_getrealtimefrommonotonictime(int64_t monotonic){
	int64_t relative = monotonic - posix_time_getcurrenttime(MICROEJ_TRUE);
	return posix_time_getcurrenttime(MICROEJ_FALSE) + relative;
}

#ifdef __cplusplus
	}
#endif
