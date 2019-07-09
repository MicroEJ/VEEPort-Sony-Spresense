
/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include "gnss_LLAPI.h"
#include "gnss_functions.h"
#include "sni.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

float GNSS_getLatitude()
{
    return gnss_get_latitude();
}
float GNSS_getLongitude()
{
    return gnss_get_longitude();
}
int GNSS_getTimeHours()
{
    return gnss_get_time_hours();
}
int GNSS_getTimeMinutes()
{
    return gnss_get_time_minutes();
}
int GNSS_getTimeSeconds()
{
    return gnss_get_time_seconds();
}
int GNSS_getTimeMicroSeconds()
{
    return gnss_get_time_microseconds();
}

int GNSS_getYear()
{
    return gnss_get_year();
}

int GNSS_getMonth()
{
    return gnss_get_month();
}

int GNSS_getDay()
{
    return gnss_get_day();
}

void GNSS_fetch()
{
    int status = gnss_fetch();
    if (status == GNSS_ERROR)
    {
        SNI_throwNativeException(GNSS_ERROR, "Read data failed");
    }
    else if (status == GNSS_INVALID_DATA)
    {
        SNI_throwNativeException(GNSS_INVALID_DATA, "No Positioning Data");
    }
}

void GNSS_initialize()
{
    if (gnss_init() != GNSS_OK)
    {
        SNI_throwNativeException(GNSS_ERROR, "Initialization failed");
    }
}
void GNSS_start()
{
    if (gnss_start() != GNSS_OK)
    {
        SNI_throwNativeException(GNSS_ERROR, "Starting GNSS failed");
    }
}
void GNSS_stop()
{
    if (gnss_stop() != GNSS_OK)
    {
        SNI_throwNativeException(GNSS_ERROR, "Stopping GNSS failed");
    }
}
