#ifndef FLASH_CONFIG_H
#define FLASH_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#define FLASH_CONFIG_MAGIC 0x50484F54

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

bool flash_config_read(wifi_config_t* config);
bool flash_config_write(const wifi_config_t* config);
bool flash_config_exists(void);
void flash_config_erase(void);

#endif  // FLASH_CONFIG_H
