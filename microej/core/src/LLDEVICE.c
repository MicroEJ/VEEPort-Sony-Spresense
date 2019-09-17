/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */


#include "LLDEVICE_impl.h"
#include "arch/board/board.h"
#include <stdint.h>
#include <string.h>

#define UNIQUE_DEVICE_ID_SIZE_IN_BYTE CONFIG_BOARDCTL_UNIQUEID_SIZE

uint8_t LLDEVICE_IMPL_getArchitecture(uint8_t* buffer, int32_t length) {
	if (length >= 6) {
		buffer[0]= 'C';
		buffer[1]= 'X';
		buffer[2]= 'D';
		buffer[3]= '5';
		buffer[4]= '6';
		buffer[5]= 0;
		return 1;
	}
	else {
		return 0;
	}
}

uint32_t LLDEVICE_IMPL_getId(uint8_t* buffer, int32_t length) {
	if(length < UNIQUE_DEVICE_ID_SIZE_IN_BYTE){
		return 0;
	}

    uint8_t UID_BASE[UNIQUE_DEVICE_ID_SIZE_IN_BYTE];
    boardctl(BOARDIOC_UNIQUEID, (uintptr_t)UID_BASE);

	memcpy(buffer, (const char*)UID_BASE, UNIQUE_DEVICE_ID_SIZE_IN_BYTE);
	return UNIQUE_DEVICE_ID_SIZE_IN_BYTE;
}
