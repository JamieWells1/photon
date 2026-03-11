/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef FLASH_MEMORY_H
#define FLASH_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

#define FLASH_MEMORY_MAGIC 0x50484F54

typedef struct
{
    uint32_t magic;
    char ssid[64];
    char password[64];

    uint8_t brightness;
    uint8_t location;
    uint8_t currency;

    uint16_t crypto;
    uint64_t stocks;

    uint8_t stock_ticker_count;
    char stock_tickers[10][10];

    uint8_t portfolio_count;
    uint8_t portfolio_mode;
    char portfolio_tickers[50][10];  // Max 50 tickers (T212 limit)
    float portfolio_quantities[50];
    float portfolio_avg_prices[50];
    float portfolio_csv_prices[50];

    char firmware_version[9];  // YYYYMMDD format + null terminator

    uint32_t crc32;
} device_config_t;

bool flash_memory_read(device_config_t* config);

bool flash_memory_write(const device_config_t* config);
void flash_memory_erase(void);

#endif  // FLASH_MEMORY_H
