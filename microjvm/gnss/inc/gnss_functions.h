
/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#ifndef GNSS_FUNCTION_H
#define GNSS_FUNCTION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <arch/chip/gnss.h>
#include <pthread.h>

#define GNSS_OK 0
#define GNSS_ERROR -1
#define GNSS_INVALID_DATA -1

typedef enum gnss_state{
    GNSS_INIT,
    GNSS_START,
    GNSS_STOP
} gnss_state;

typedef struct gnss_info {
    int fd;
    gnss_state state;
    struct cxd56_gnss_positiondata_s posdat;
    sigset_t mask;
    volatile bool running;
    volatile bool data_ready;
    pthread_t signaling_thread;
    pthread_mutex_t mutex;
} gnss_info;

int gnss_stop(void);
int gnss_close(void);
int gnss_read(void);
void gnss_signo(void);
int gnss_fetch(void);
int gnss_setparams(void);
int gnss_init(void);
bool gnss_start(void);

float gnss_get_latitude(void);
float gnss_get_longitude(void);
int gnss_get_time_hours(void);
int gnss_get_time_minutes(void);
int gnss_get_time_seconds(void);
int gnss_get_time_microseconds(void);
int gnss_get_year(void);
int gnss_get_month(void);
int gnss_get_day(void);


int32_t microej_list_thread_id_compare(void* node, void* thread_id);
void microej_list_thread_id_delete(void* item);
void microej_list_thread_id_consume(void* item);


#ifdef __cplusplus
}
#endif

#endif /* GNSS_FUNCTION_H */