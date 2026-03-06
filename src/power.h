#ifndef POWER_H
#define POWER_H

#include <stdint.h>

// Public API - logs power consumption stats at TRACE level
void power_log_stats(uint8_t brightness_pct);

#endif // POWER_H
