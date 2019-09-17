
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
float GNSS_getAltitude()
{
    return gnss_get_altitude();
}
float GNSS_getVelocity()
{
    return gnss_get_velocity();
}
int64_t GNSS_getTime()
{
    return gnss_get_time();
}

void GNSS_fetch()
{
    int status = gnss_fetch();
    if (status == GNSS_ERROR)
    {
        SNI_throwNativeIOException(GNSS_ERROR, "Read data failed");
    }
    else if (status == GNSS_INVALID_DATA)
    {
        SNI_throwNativeIOException(GNSS_INVALID_DATA, "No satellite signal");
    }
}

void GNSS_initialize()
{
    if(gnss_init() != GNSS_OK)
    {
        SNI_throwNativeException(GNSS_ERROR, "GNSS Initialization failed");
    }
}

void GNSS_start()
{
    if (gnss_start() != GNSS_OK)
    {
        SNI_throwNativeIOException(GNSS_ERROR, "Starting GNSS failed");
    }
}
void GNSS_stop()
{
    if (gnss_stop() != GNSS_OK)
    {
        SNI_throwNativeIOException(GNSS_ERROR, "Stopping GNSS failed");
    }
}

void GNSS_close()
{
    if (gnss_close() != GNSS_OK)
    {
        SNI_throwNativeIOException(GNSS_ERROR, "Closing GNSS failed");
    }
}
