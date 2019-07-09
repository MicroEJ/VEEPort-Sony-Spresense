/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include <math.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <sched.h>

#include <nuttx/init.h>
#include <nuttx/clock.h>
#include "arch/board/board.h"
#include "microej.h"
#include "posix_time.h"
#include "LLMJVM_impl.h"
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/** True if the VM has been waken up */
	static sem_t mutex;
	pid_t VM_PID;
	static volatile systime_t nextWakeUpTicks = UINT32_MAX;
	volatile uint8_t vm_is_running = 0;
	volatile int64_t test_counter = 0;

	/*
	 * Implementation of the timerhook defined by CONFIG_SYSTEMTICK_HOOK in the NuttX Kernel
	 */

	void board_timerhook()
	{
		if (vm_is_running && nextWakeUpTicks <= clock_systimer())
		{
			LLMJVM_schedule();
		}
	}
	/*
 * Implementation of functions from LLMJVM_impl.h
 * and other helping functions.
 */
	int32_t LLMJVM_IMPL_initialize()
	{
		int result = sem_init(&mutex, 0, 0); // initialize semaphor
		assert(result == 0);
		// init timer
		vm_is_running = 1;
		VM_PID = getpid();
		if (result != 0)
		{
			return LLMJVM_ERROR;
		}
		posix_time_initialize_system_time_offset();
		return LLMJVM_OK;
	}

	// Once the task is started, save a handle to it.
	int32_t LLMJVM_IMPL_vmTaskStarted()
	{
		return LLMJVM_OK;
	}

	// Schedules requests from the VM
	int32_t LLMJVM_IMPL_scheduleRequest(int64_t absoluteTime)
	{

		if (absoluteTime <= LLMJVM_IMPL_getCurrentTime(MICROEJ_TRUE))
		{
			LLMJVM_schedule();
			nextWakeUpTicks = UINT32_MAX;
		}
		else
		{
			nextWakeUpTicks = absoluteTime * TICK_PER_MSEC;
		}
		return LLMJVM_OK;
	}

	// Suspends the VM task if the pending flag is not set
	int32_t LLMJVM_IMPL_idleVM()
	{
		while (sem_wait(&mutex) != 0)
		{
			if (errno == EINTR)
			{
				errno = 0;
			}
		}
		return LLMJVM_OK;
	}

	// Wakes up the VM task and reset next wake up time
	int32_t LLMJVM_IMPL_wakeupVM()
	{
		// notify thread
		int value;
		int ret = sem_getvalue(&mutex, &value);
		if (ret == OK && value <= 0)
		{
			nextWakeUpTicks = UINT32_MAX;
			int32_t result = sem_post(&mutex);
			return (result == 0) ? LLMJVM_OK : LLMJVM_ERROR;
		}
		else
		{
			return LLMJVM_OK;
		}
	}

	// Clear the pending wake up flag
	int32_t LLMJVM_IMPL_ackWakeup()
	{
		return LLMJVM_OK;
	}

	int32_t LLMJVM_IMPL_getCurrentTaskID()
	{
		return VM_PID;
	}

	void LLMJVM_IMPL_setApplicationTime(int64_t t)
	{
		posix_time_setapplicationtime(t);
	}

	// Gets the system or the application time in milliseconds
	int64_t LLMJVM_IMPL_getCurrentTime(uint8_t isPlatformTime)
	{
		int64_t curr_time = posix_time_getcurrenttime(isPlatformTime);
		return curr_time;
	}

	// Gets the current system time in nanoseconds
	int64_t LLMJVM_IMPL_getTimeNanos()
	{
		return posix_time_gettimenanos();
	}

	extern void lcd_finalize();
	int32_t LLMJVM_IMPL_shutdown()
	{
		vm_is_running = 0;
		int32_t result;
		assert(result == 0);
		result = sem_destroy(&mutex);
		assert(result == 0);
		return LLMJVM_OK;
	}

#ifdef __cplusplus
}
#endif