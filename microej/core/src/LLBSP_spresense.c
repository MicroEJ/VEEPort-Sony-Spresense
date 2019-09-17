/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include "LLBSP_impl.h"
#include "microej.h"
#include <stdio.h>

#define MEM_SRAM_START 			(0x0d000000)
#define MEM_SRAM_END 			(0x0d180000)

/* Public functions ----------------------------------------------------------*/

// Checks if the given pointer is in a read only memory or not.
uint8_t LLBSP_IMPL_isInReadOnlyMemory(void *ptr)
{
	if ((uint32_t)ptr >= MEM_SRAM_START && (uint32_t)ptr <= MEM_SRAM_END)
	{
		return MICROEJ_FALSE;
	}
	else
	{
		return MICROEJ_TRUE;
	}
}

/**
 * Writes the character <code>c</code>, cast to an unsigned char, to stdout stream.
 * This function is used by the default implementation of the Java <code>System.out</code>.
 */

void LLBSP_IMPL_putchar(int32_t c)
{
	fputc(c, stdout);
}