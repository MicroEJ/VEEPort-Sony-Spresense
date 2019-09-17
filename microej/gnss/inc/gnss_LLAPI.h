/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#ifndef GNSS_LLAPI_H
#define GNSS_LLAPI_H


#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

#define GNSS_fetch                  Java_ej_gnss_GnssMicroEJNative_GNSS_1fetchData
#define GNSS_getTime                Java_ej_gnss_GnssMicroEJNative_GNSS_1getTime
#define GNSS_getLongitude           Java_ej_gnss_GnssMicroEJNative_GNSS_1getLongitude
#define GNSS_getLatitude            Java_ej_gnss_GnssMicroEJNative_GNSS_1getLatitude
#define GNSS_getAltitude            Java_ej_gnss_GnssMicroEJNative_GNSS_1getAltitude
#define GNSS_getVelocity            Java_ej_gnss_GnssMicroEJNative_GNSS_1getVelocity
#define GNSS_initialize             Java_ej_gnss_GnssMicroEJNative_GNSS_1initialize
#define GNSS_start                  Java_ej_gnss_GnssMicroEJNative_GNSS_1start
#define GNSS_close                  Java_ej_gnss_GnssMicroEJNative_GNSS_1close
#define GNSS_stop                   Java_ej_gnss_GnssMicroEJNative_GNSS_1stop


float GNSS_getLatitude(void);
float GNSS_getLongitude(void);
float GNSS_getAltitude(void);
float GNSS_getVelocity(void);
int64_t GNSS_getTime(void);
void GNSS_fetch(void);
void GNSS_initialize(void);
void GNSS_start(void);
void GNSS_stop(void);
void GNSS_close(void);

#ifdef __cplusplus
}
#endif

#endif /* GNSS_LLAPI_H*/