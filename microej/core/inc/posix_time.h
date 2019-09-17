/*
 * C
 *
 * Copyright 2019 IS2T. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break IS2T warranties on the whole library.
 */

#ifndef _POSIX_TIME_H
#define _POSIX_TIME_H

/**
 * @file
 * @brief POSIX time API.
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif


int64_t posix_time_getcurrenttime(uint8_t isPlatformTime);
int64_t posix_time_gettimenanos();
void posix_time_setapplicationtime(int64_t t);
int64_t posix_time_getrealtimefrommonotonictime(int64_t monotonic);


#ifdef __cplusplus
	}
#endif

#endif // _POSIX_TIME_H

