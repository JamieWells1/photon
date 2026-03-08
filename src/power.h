#ifndef POWER_H
#define POWER_H

#include <const.h>

#include <stdint.h>

// Public API - logs power consumption stats at TRACE level
void power_log_stats(LogLevel log_level, uint8_t brightness_pct);

#endif  // POWER_H
