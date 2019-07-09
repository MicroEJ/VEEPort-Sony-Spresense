/*
 * C
 *
 * Copyright 2019 MicroEJ Corp. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break MicroEJ Corp. warranties on the whole library.
 */

/**
 * @file
 * @brief Asynchronous Worker implementation
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

#include "microej_async_worker.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// Entry point of the async worker task.
	static void *MICROEJ_ASYNC_WORKER_loop(void *args);

	MICROEJ_ASYNC_WORKER_status_t MICROEJ_ASYNC_WORKER_initialize(MICROEJ_ASYNC_WORKER_handle_t *worker, uint8_t *name, OSAL_task_stack_t stack, int32_t priority)
	{
		// Check configuration
		int32_t job_count = worker->job_count;
		if (job_count <= 0 || worker->waiting_threads_length <= 0)
		{
			return MICROEJ_ASYNC_WORKER_INVALID_ARGS;
		}

		// Init jobs
		MICROEJ_ASYNC_WORKER_job_t *jobs = worker->free_jobs;
		void *params = worker->params;
		int32_t params_sizeof = worker->params_sizeof;
		for (int i = 0; i < job_count - 1; i++)
		{
			jobs[i]._intern.next_free_job = &jobs[i + 1];
			jobs[i].params = params;
			params = params + params_sizeof;
		}
		jobs[job_count - 1]._intern.next_free_job = NULL;
		jobs[job_count - 1].params = params;

		// Create queue
		OSAL_status_t res = OSAL_queue_create(name, worker->job_count, &worker->jobs_queue);
		if (res != OSAL_OK)
		{
			return MICROEJ_ASYNC_WORKER_ERROR;
		}

		// Create task
		res = OSAL_task_create(MICROEJ_ASYNC_WORKER_loop, name, stack, priority, worker, &worker->task);
		if (res != OSAL_OK)
		{
			return MICROEJ_ASYNC_WORKER_ERROR;
		}
		return MICROEJ_ASYNC_WORKER_OK;
	}

	MICROEJ_ASYNC_WORKER_job_t *MICROEJ_ASYNC_WORKER_allocate_job(MICROEJ_ASYNC_WORKER_handle_t *async_worker, SNI_callback sni_retry_callback)
	{

		MICROEJ_ASYNC_WORKER_job_t *job = async_worker->free_jobs;
		if (job == NULL)
		{
			// No free job available: wait for a free job.
			// Store the current thread id in the waiting list.
			// First check if there is a free element in the waiting list.
			int32_t free_waiting_thread_offset = async_worker->free_waiting_thread_offset;
			int32_t new_free_waiting_thread_offset = free_waiting_thread_offset + 1;
			if (new_free_waiting_thread_offset >= async_worker->waiting_threads_length)
			{
				new_free_waiting_thread_offset = 0;
			}

			if (new_free_waiting_thread_offset == async_worker->waiting_thread_offset)
			{
				// The waiting list is full.
				SNI_throwNativeIOException(-1, "MICROEJ_ASYNC_WORKER: thread cannot be suspended, waiting list is full.");
			}
			else
			{
				async_worker->free_waiting_thread_offset = (uint16_t)new_free_waiting_thread_offset;
				int32_t thread_id = SNI_getCurrentJavaThreadID();
				async_worker->waiting_threads[free_waiting_thread_offset] = thread_id;
				SNI_suspendCurrentJavaThreadWithCallback(0, sni_retry_callback, NULL);
			}

			return NULL;
		}
		else
		{
			// Free job found: remove it from the free list and return it.
			async_worker->free_jobs = job->_intern.next_free_job;
			job->_intern.next_free_job = NULL;
			return job;
		}
	}

	MICROEJ_ASYNC_WORKER_status_t MICROEJ_ASYNC_WORKER_free_job(MICROEJ_ASYNC_WORKER_handle_t *worker, MICROEJ_ASYNC_WORKER_job_t *job)
	{
		job->_intern.next_free_job = worker->free_jobs;
		worker->free_jobs = job;

		int32_t waiting_thread_offset = worker->waiting_thread_offset;
		if (waiting_thread_offset != worker->free_waiting_thread_offset)
		{
			// A thread was waiting for a free job: notify it
			int32_t thread_id = worker->waiting_threads[waiting_thread_offset];
			int32_t new_waiting_thread_offset = waiting_thread_offset + 1;
			if (new_waiting_thread_offset >= worker->waiting_threads_length)
			{
				new_waiting_thread_offset = 0;
			}
			worker->waiting_thread_offset = new_waiting_thread_offset;
			SNI_resumeJavaThread(thread_id);
		}

		return MICROEJ_ASYNC_WORKER_OK;
	}

	MICROEJ_ASYNC_WORKER_status_t MICROEJ_ASYNC_WORKER_async_exec(MICROEJ_ASYNC_WORKER_handle_t *worker, MICROEJ_ASYNC_WORKER_job_t *job, MICROEJ_ASYNC_WORKER_action_t action, SNI_callback on_done_callback)
	{

		job->_intern.action = action;
		job->_intern.thread_id = SNI_getCurrentJavaThreadID();
		OSAL_status_t res = OSAL_queue_post(&worker->jobs_queue, job);
		if (res == OSAL_OK)
		{
			SNI_suspendCurrentJavaThreadWithCallback(0, on_done_callback, job);
			return MICROEJ_ASYNC_WORKER_OK;
		}
		else
		{
			SNI_throwNativeIOException(-1, "MICROEJ_ASYNC_WORKER: Internal error.");
			return MICROEJ_ASYNC_WORKER_ERROR;
		}
	}

	MICROEJ_ASYNC_WORKER_job_t *MICROEJ_ASYNC_WORKER_get_job_done()
	{
		MICROEJ_ASYNC_WORKER_job_t *job = NULL;
		SNI_getCallbackArgs((void **)&job, NULL);
		return job;
	}

	static void *MICROEJ_ASYNC_WORKER_loop(void *args)
	{
		MICROEJ_ASYNC_WORKER_handle_t *worker = (MICROEJ_ASYNC_WORKER_handle_t *)args;

		while (1)
		{
			MICROEJ_ASYNC_WORKER_job_t *job;
			OSAL_status_t res = OSAL_queue_fetch(&worker->jobs_queue, (void **)&job, OSAL_INFINITE_TIME);

			if (res == OSAL_OK)
			{
				// New job to execute
				job->_intern.action(job);
				SNI_resumeJavaThread(job->_intern.thread_id);
			}
		}
	}

#ifdef __cplusplus
}
#endif
