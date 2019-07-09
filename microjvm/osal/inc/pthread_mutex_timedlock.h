/*
 * C
 *
 * Copyright 2019 IS2T. All rights reserved
 * This library is provided in source code for use, modification and test, subject to license terms
 * Any modification of the source code will break IS2T warranties on the whole library
 */


#include <pthread.h>
#include <time.h>

int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec *restrict abs_timeout);
