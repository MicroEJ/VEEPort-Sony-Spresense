/*
 * C
 *
 * Copyright 2019 IS2T. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break IS2T warranties on the whole library.
 */

#ifndef __MICROEJ_H
#define __MICROEJ_H

/**
 * @file
 * @brief Generic define for MicroEJ.
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif


#define MICROEJ_FALSE 	0
#define MICROEJ_TRUE 	1
/**
 * @brief Starts the MicroJvm virtual machine and executes its Java main method.
 *
 * @param[in] argc number of string arguments in <code>argv</code>.
 * @param[in] array of string arguments given to the Java main() method. May be null.
 *
 * @return the Java application exit code or the MicroJvm virtual machine error code on error.
 */
int microjvm_start(int argc, char* argv[]);



#ifdef __cplusplus
	}
#endif

#endif

