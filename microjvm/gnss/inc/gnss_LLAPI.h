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

#define GNSS_fetch                  Java_com_microej_gnss_GnssMicroEJNative_GNSS_1fetchData
#define GNSS_getTimeYear            Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeYear
#define GNSS_getTimeMonth           Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeMonth
#define GNSS_getTimeDay                Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeDay
#define GNSS_getTimeHours           Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeHours
#define GNSS_getTimeMinutes         Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeMinutes
#define GNSS_getTimeSeconds         Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeSeconds
#define GNSS_getTimeMicroSeconds    Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getTimeMicroSeconds
#define GNSS_getLongitude           Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getLongitude
#define GNSS_getLatitude          Java_com_microej_gnss_GnssMicroEJNative_GNSS_1getLattitude
#define GNSS_initialize             Java_com_microej_gnss_GnssMicroEJNative_GNSS_1initialize
#define GNSS_start                  Java_com_microej_gnss_GnssMicroEJNative_GNSS_1start

float GNSS_getLatitude(void);
float GNSS_getLongitude(void);
int GNSS_getYear(void);
int GNSS_getMonth(void);
int GNSS_getDay(void);
int GNSS_getTimeHours(void);
int GNSS_getTimeMinutes(void);
int GNSS_getTimeSeconds(void);
int GNSS_getTimeMicroSeconds(void);
void GNSS_fetch(void);
void GNSS_initialize(void);
void GNSS_start(void);
void GNSS_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* GNSS_LLAPI_H*/