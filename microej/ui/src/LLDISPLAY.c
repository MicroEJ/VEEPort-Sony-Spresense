/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include <sdk/config.h>
#include <stdio.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/spi/spi.h>
#include <nuttx/lcd/lcd.h>
#include <nuttx/lcd/ili9340.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <arch/board/board.h>
#include <arch/chip/pin.h>

#include "LLDISPLAY_impl.h"
#include "LLDISPLAY_EXTRA_impl.h"

struct fb_videoinfo_s vinfo;
struct lcd_planeinfo_s pinfo;
struct lcd_dev_s *dev;

#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_BPP 16
uint8_t DISPLAY_BUFFER[LCD_WIDTH * LCD_HEIGHT * LCD_BPP / 8];

volatile uint8_t microui_is_running = 0;
static volatile int y_min, y_max;

sem_t flush_sem;
sem_t sync_sem;

// There is no place to call finalize currently
void lcd_finalize()
{
    if (microui_is_running)
    {
        microui_is_running = 0;
        sem_post(&flush_sem);
        int result = sem_destroy(&flush_sem);
        result = sem_destroy(&sync_sem) || result;
        if (result != 0)
        {
            puts("Error during lcd_finalize");
        }
    }
}

// This function takes about 40 ms
static void lcd_flush(void)
{
    for (int i = y_min; i <= y_max; ++i)
    {
        pinfo.putrun(i, 0, DISPLAY_BUFFER + (i * LCD_WIDTH) * (LCD_BPP / 8), LCD_WIDTH);
    }
    int value;
    sem_getvalue(&sync_sem, &value);
    if (value <= 0)
    {
        sem_post(&sync_sem);
    }
}

static void lcd_memcpy_func(void)
{
    while (microui_is_running)
    {
        while (sem_wait(&flush_sem) != 0)
        {
            if (errno == EINTR)
            {
                errno = 0;
            }
        }
        if (microui_is_running)
        {
            lcd_flush();
        }
        else
        {
            break;
        }
    }
}

void LLDISPLAY_IMPL_initialize()
{
    int result = board_lcd_initialize();

    dev = board_lcd_getdev(0);
    dev->setpower(dev, CONFIG_LCD_MAXPOWER);

    result = dev->getvideoinfo(dev, &vinfo);
    assert(result == 0);
    result = dev->getplaneinfo(dev, 0, &pinfo);
    assert(result == 0);

    assert(LCD_WIDTH == vinfo.xres);
    assert(LCD_HEIGHT == vinfo.yres);
    assert(LCD_BPP == pinfo.bpp);

    result = sem_init(&flush_sem, 0, 0);
    assert(result == 0);
    result = sem_init(&sync_sem, 0, 0);
    assert(result == 0);
    microui_is_running = 1;

    task_create("flush_display", 100, 512, lcd_memcpy_func, NULL);
}

int32_t LLDISPLAY_IMPL_getWidth()
{
    return vinfo.xres;
}

int32_t LLDISPLAY_IMPL_getHeight()
{
    return vinfo.yres;
}

int32_t LLDISPLAY_IMPL_getGraphicsBufferAddress()
{
    return (uint32_t)DISPLAY_BUFFER;
}

void LLDISPLAY_IMPL_synchronize()
{
    while (sem_wait(&sync_sem) != 0)
    {
        if (errno == EINTR)
        {
            errno = 0;
        }
    }
}

int32_t LLDISPLAY_IMPL_flush(int32_t sourceAddr, int32_t xmin, int32_t ymin, int32_t xmax, int32_t ymax)
{
    y_min = ymin;
    y_max = ymax;
    int value;
    sem_getvalue(&flush_sem, &value);
    if (value <= 0)
    {
        sem_post(&flush_sem);
    }

    return sourceAddr;
}

int32_t LLDISPLAY_EXTRA_IMPL_hasBackLight(void)
{
    return LLDISPLAY_EXTRA_OK;
}

void LLDISPLAY_EXTRA_IMPL_backlightOn(void)
{
    dev->setpower(dev, CONFIG_LCD_MAXPOWER);
}

void LLDISPLAY_EXTRA_IMPL_backlightOff(void)
{
    dev->setpower(dev, 0);
}