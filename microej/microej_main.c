/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include "microej.h"
#include <stdint.h>
#include <stdio.h>
#include <sched.h>
#include <sdk/config.h>
#include <nuttx/init.h>
#include "arch/board/board.h"
#include "platform/cxxinitialize.h"
#include <dirent.h>
#include "sni.h"
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>

#include "LLFS_File_impl.h"
#include "LLFS_impl.h"

extern void lcd_finalize(void);

int microej_main(int argc, char *argv[])
{
#if (CONFIG_SDK_USER_ENTRYPOINT == microej_main)
    // Should be called if entry points is microej instead of the default you remove NSH
    int r = boardctl(BOARDIOC_INIT, 0);
    if (r)
    {
        puts("WARNING: Something wrong during board initialization");
    }

    int ret;
    int retry = 0;
    struct stat buf;

    for (retry = 0; retry < 20; retry++)
    {
        ret = stat("/mnt/sd0", &buf);
        if (ret == 0)
        {
            break;
        }
        usleep(100 * 1000); // 100 msec
    }
#endif

    /* MicroEJ VM launch */
    microjvm_start(argc, argv);

    /* Clean up phase */
    /* Not necessary if you never return from the VM but it's cleaner to properly dispose of the ressources */
    lcd_finalize();
    return 0;
}
