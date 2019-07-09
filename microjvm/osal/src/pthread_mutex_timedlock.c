/*
 * C
 *
 * Copyright 2019 IS2T. All rights reserved
 * This library is provided in source code for use, modification and test, subject to license terms
 * Any modification of the source code will break IS2T warranties on the whole library
 */

#include "pthread_mutex_timedlock.h"
#include <sched.h>
#include "errno.h"

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *restrict abs_timeout)
{
    int mypid = (int)getpid();
    int ret = EINVAL;
    if (mutex != NULL)
    {
        /* Make sure the semaphore is stable while we make the following
       * checks.  This all needs to be one atomic action.
       */
        sched_lock();

        /* Error out if the mutex is already in an inconsistent state. */

        if ((mutex->flags & _PTHREAD_MFLAGS_INCONSISTENT) != 0)
        {
            ret = EOWNERDEAD;
        }
        else
        {
            /* Take the semaphore */
            while (sem_timedwait(&mutex->sem, abs_timeout))
            {
                if (errno == EINTR)
                {
                    errno = 0;
                }
                else if(errno == ETIMEDOUT)
                {
                    errno = 0;
                    ret = ETIMEDOUT;
                    break;
                }
            }

            /* If we successfully obtained the semaphore, then indicate
             * that we own it.
             */
            if (ret == OK)
            {
                mutex->pid = mypid;
            }
        }
        sched_unlock();
    }
    return ret;
}
