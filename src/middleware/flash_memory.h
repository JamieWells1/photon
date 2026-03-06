#ifndef FLASH_MEMORY_H
#define FLASH_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

#define FLASH_MEMORY_MAGIC 0x50484F54

// Default configuration values
#define DEFAULT_BRIGHTNESS 8      // 8%
#define DEFAULT_LOCATION 0        // London
#define DEFAULT_CURRENCY 0        // GBP

typedef struct
{
    uint32_t magic;
    char ssid[64];
    char password[64];

    uint8_t brightness;
    uint8_t location;
    uint8_t currency;

    uint32_t crc32;
} device_config_t;

typedef device_config_t wifi_config_t;

bool flash_memory_read(wifi_config_t* config);
void flash_memory_read_with_defaults(wifi_config_t* config);
bool flash_memory_write(const wifi_config_t* config);
bool flash_memory_exists(void);
void flash_memory_erase(void);

#endif  // FLASH_MEMORY_H
