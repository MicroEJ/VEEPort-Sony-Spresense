/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#ifdef __cplusplus
extern "C"
{
#endif
#include <semaphore.h>
#include <assert.h>
#include "LLSP_impl.h"

    static sem_t rw_access_mutex;
    static sem_t task_mutex;
    /**
 * Initialize Shielded Plug synchronization data.
 * This function may be called several times. It is not called from a thread safe context so several threads
 * may call this method at the same time. The implementation may use synchronization mechanisms to ensure
 * that the initialization is made once.
 */
    void LLSP_IMPL_initialize(void)
    {
        int result = sem_init(&rw_access_mutex, 0, 1); // initialize semaphore
        assert(result == 0);
        result = sem_init(&rw_access_mutex, 0, 1); // initialize semaphore
        assert(result == 0);
        return LLSP_OK;
    }

    /**
 * This function is called before writing data in a block.
 * It must implement synchronization mechanisms to avoid multiple write access to the given block.
 */
    void LLSP_IMPL_syncWriteBlockEnter(int32_t databaseID, int32_t blockID)
    {
        sem_wait(&rw_access_mutex);
    }

    /**
 * This function is called after writing data in a block.
 * It must implement synchronization mechanisms to avoid multiple write access to the given block.
 */
    void LLSP_IMPL_syncWriteBlockExit(int32_t databaseID, int32_t blockID)
    {
        int value;
        sem_getvalue(&rw_access_mutex, &value);
        if (value <= 0)
        {
            sem_post(&rw_access_mutex);
        }
    }

    /**
 * This function is called before reading data from a block.
 * It must implement synchronization mechanisms to avoid write access while reading the given block.
 */
    void LLSP_IMPL_syncReadBlockEnter(int32_t databaseID, int32_t blockID)
    {
        sem_wait(&rw_access_mutex);
    }

    /**
 * This function is called after reading data from a block.
 * It must implement synchronization mechanisms to avoid write access while reading the given block.
 */
    void LLSP_IMPL_syncReadBlockExit(int32_t databaseID, int32_t blockID)
    {
        int value;
        sem_getvalue(&rw_access_mutex, &value);
        if (value <= 0)
        {
            sem_post(&rw_access_mutex);
        }
    }

    /**
 * Blocks the current task until {@link LLSP_wakeup} is called or the current task is interrupted.
 * @return {@link LLSP_OK} if the current task has been woken up, or {@link LLSP_INTERRUPTED} if the current task has been interrupted.
 */
    int32_t LLSP_IMPL_wait(void)
    {
        sem_wait(&task_mutex);
        return LLSP_OK;
    }

    /**
 * Wake up the specified given task (which is waiting in {@link LLSP_wait}).
 */
    void LLSP_IMPL_wakeup(int32_t taskID)
    {
        int value;
        sem_getvalue(&task_mutex, &value);
        if (value <= 0)
        {
            sem_post(&task_mutex);
        }
    }

#ifdef __cplusplus
}
#endif