/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include <stdio.h>
#include "LLLEDS_impl.h"
#include "arch/board/board.h"
#include "arch/chip/pin.h"

#define PINTYPE_LED 0x60
#define LED_PIN_CONVERT(N) (uint8_t)(PINTYPE_LED | (N))
#define NUM_LEDS 4
// --------------------------------------------------------------------------------
// -                      Functions that must be implemented                      -
// --------------------------------------------------------------------------------
/**
 * Initializes the LEDs.
 * @return the number of LEDs in the group.
 */
int32_t LLLEDS_IMPL_initialize()
{
    for (int i = 1; i <= NUM_LEDS; ++i)
    {
        board_gpio_write(LED_PIN_CONVERT(i), -1);
        board_gpio_config(LED_PIN_CONVERT(i), 0, /* input = */ false, /* highdrive = */ true, PIN_FLOAT);
    }
    return NUM_LEDS;
}

/**
 * Get the intensity of the LED.
 * @return the intensity of the LED
 */
int32_t LLLEDS_IMPL_getIntensity(int32_t ledID)
{
    assert(ledID >= 1 && ledID <= 4);
    return board_gpio_read(LED_PIN_CONVERT(ledID));
}

/**
 * Set the intensity of the LED.
 * A 0 intensity turns off the LED, intensity of 255 turns on the LED to max power.
 * @param intensity the intensity of the LED
 */
void LLLEDS_IMPL_setIntensity(int32_t ledID, int32_t intensity)
{
    assert(ledID >= 1 && ledID <= 4);
    board_gpio_write(LED_PIN_CONVERT(ledID), (bool)intensity);
}