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
    uint16_t stocks;

    uint8_t portfolio_count;
    char portfolio_tickers[30][10];
    float portfolio_quantities[30];
    float portfolio_avg_prices[30];

    uint32_t crc32;
} device_config_t;

bool flash_memory_read(device_config_t* config);

bool flash_memory_write(const device_config_t* config);
void flash_memory_erase(void);

#endif  // FLASH_MEMORY_H
