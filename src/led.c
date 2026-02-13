#include <blink.pio.h>
#include <hardware/pio.h>
#include <led.h>
#include <pico/stdlib.h>
#include <stdio.h>

static PIO pio = NULL;
static uint sm = 0;
static uint offset = 0;

int led_init(void)
{
    pio = pio0;
    sm = 0;

    offset = pio_add_program(pio, &blink_program);
    printf("LED PIO program loaded at offset %d\n", offset);

    return offset;
}

void led_blink_start(uint pin, uint freq)
{
    if (pio == NULL)
    {
        printf("Error: LED system not initialized\n");
        return;
    }

    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}
