#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include <pico/time.h>

uint64_t get_current_time_ms(void) { return to_ms_since_boot(get_absolute_time()); }

#endif  // UTILS_H
