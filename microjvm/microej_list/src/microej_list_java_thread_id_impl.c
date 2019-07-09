
/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include "microej_list.h"

int32_t microej_list_thread_id_compare(void *node, void *thread_id)
{
    if ((int)(thread_id) == (int)(node))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void microej_list_thread_id_delete(void *item)
{
    return;
}

void microej_list_thread_id_consume(void *item)
{
    SNI_resumeJavaThread((int)(item));
}
