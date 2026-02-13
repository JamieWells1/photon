#ifndef LED_H
#define LED_H

#include <hardware/pio.h>

/**
 * Initialize the LED blinking system
 * @return The PIO instruction memory offset, or -1 on failure
 */
int led_init(void);

/**
 * Start blinking an LED on a specific pin
 * @param pin GPIO pin number
 * @param freq Frequency in Hz
 */
void led_blink_start(uint pin, uint freq);

#endif // LED_H
