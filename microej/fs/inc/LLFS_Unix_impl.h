/*
 * C
 *
 * Copyright 2016-2019 MicroEJ Corp. All rights reserved.
 * MicroEJ Corp. PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef LLFS_UNIX_IMPL
#define LLFS_UNIX_IMPL

/**
 * @file
 * @brief MicroEJ FS Unix low level API
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

#include <stdint.h>
#include <intern/LLFS_Unix_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Convert a pathname to canonical form.  The input path (path) is assumed to contain
 * no duplicate slashes. The result is stored in canonicalizePath.
 * On POSIX we can use realpath() to do this work.<p>
 * This method may not throw a NativeIOException if the file referenced by the given path
 * does not exist.
 *
 * @param path
 * 			path to canonicalize
 *
 * @param canonicalizePath
 * 			buffer to fill with the canonicalized path
 *
 * @param canonicalizePathLength
 * 			length of canonicalizePath
 *
 * @note Throws NativeIOException on error.
 *
 * @warning path and canonicalizePath must not be used outside of the VM task or saved.
 */
void LLFS_Unix_IMPL_canonicalize(uint8_t* path, uint8_t* canonicalizePath, int32_t canonicalizePathLength);


#ifdef __cplusplus
}
#endif

#endif
