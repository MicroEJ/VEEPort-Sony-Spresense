/*
 * C
 *
 * Copyright 2016-2019 MicroEJ Corp. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break MicroEJ Corp. warranties on the whole library.
 */

/**
 * @file
 * @brief LLFS implementation over POSIX API.
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

/* Includes ------------------------------------------------------------------*/

#include "LLFS_Unix_impl.h"
#include "LLFS_impl.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "sni.h"

#ifdef __cplusplus
	extern "C" {
#endif

/* Public API ----------------------------------------------------------------*/

void LLFS_Unix_IMPL_canonicalize(uint8_t* path, uint8_t* canonicalizePath, int32_t canonicalizePathLength)
{
	// This implementation uses realpath() to canonicalize the given path.
	// If canonicalizePathLength is higher than PATH_MAX, then we can fill
	// directly canonicalizePath with realpath(), otherwise we need to
	// allocate a temporary buffer.

	if(canonicalizePathLength >= PATH_MAX)
	{	// There is enough space to put the result of realpath() into canonicalizePath
		char* e_realpath = realpath(path, canonicalizePath);
		if(e_realpath == NULL)
		{
			SNI_throwNativeIOException(LLFS_NOK, "realpath: Internal error");
		}
	}
	else
	{	// There is NOT enough space to put the result of realpath() into canonicalizePath.
		// Call realpath() with a null dest buffer, it will allocate a new buffer.
		char* e_realpath = realpath(path, NULL);
		if(e_realpath != NULL)
		{
			// Check if the length of the path returned by realpath()
			// is lower than canonicalizePath length.
			// Note: use '<' and not '<=' to keep space for the final '\0'.
			if(strlen(e_realpath) < canonicalizePathLength)
			{
				strcpy(canonicalizePath, e_realpath);
			}
			else
			{
				SNI_throwNativeIOException(LLFS_NOK, "canonicalPath length too small");
			}

			// e_realpath has been "malloced" by realpath(). Free it.
			free(e_realpath);
		}
		else
		{
			SNI_throwNativeIOException(LLFS_NOK, "realpath: Internal error");
		}
	}
}

#ifdef __cplusplus
	}
#endif
