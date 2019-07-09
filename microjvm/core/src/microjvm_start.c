/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include <stdio.h>
#include "LLMJVM.h"
#include "sni.h"

#ifdef __cplusplus
	extern "C" {
#endif

/**
 * @brief Starts the MicroJvm virtual machine and executes its Java main method.
 *
 * @param[in] argc number of string arguments in <code>argv</code>.
 * @param[in] array of string arguments given to the Java main() method. May be null.
 *
 * @return the Java application exit code or the MicroJvm virtual machine error code on error.
 */
int microjvm_start(int argc, char* argv[])
{
	void* vm;
	int32_t err;
	int32_t exitcode = -1;

	// create VM
	printf("Create VM\n");
	vm = SNI_createVM();

	if(vm == NULL)
	{
		printf("VM initialization error.\n");
	}
	else
	{
		printf("VM START\n");
		err = SNI_startVM(vm, 0, NULL);

		if(err < 0)
		{
			// Error occurred
			if(err == LLMJVM_E_EVAL_LIMIT)
			{
				printf("Evaluation limits reached.\n");
			}
			else
			{
				printf("VM execution error (err = %d).\n", err);
			}
			exitcode = err;
		}
		else
		{
			// VM execution ends normally
			exitcode = SNI_getExitCode(vm);
			printf("VM END (exit code = %d)\n", exitcode);
		}

		// delete VM
		SNI_destroyVM(vm);
	}
	return exitcode;
}
#ifdef __cplusplus
	}
#endif
