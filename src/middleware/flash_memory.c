#include <flash_memory.h>

#include <const.h>
#include <macro.h>

#include <stdio.h>
#include <string.h>

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/stdlib.h>

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static uint32_t crc32(const uint8_t* data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

static void flash_memory_set_defaults(device_config_t* config)
{
    memset(config, 0, sizeof(device_config_t));
    config->brightness = DEFAULT_BRIGHTNESS;
    config->location = DEFAULT_LOCATION;
    config->currency = DEFAULT_CURRENCY;
    config->tickers = DEFAULT_TICKERS;
}

bool flash_memory_read(device_config_t* config)
{
    if (!config)
    {
        return false;
    }

    const uint8_t* flash_ptr = (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(config, flash_ptr, sizeof(device_config_t));

    if (config->magic != FLASH_MEMORY_MAGIC)
    {
        flash_memory_set_defaults(config);
        return false;
    }

    uint32_t stored_crc = config->crc32;
    config->crc32 = 0;
    uint32_t calculated_crc = crc32((const uint8_t*)config, sizeof(device_config_t));
    config->crc32 = stored_crc;

    if (stored_crc != calculated_crc)
    {
        flash_memory_set_defaults(config);
        return false;
    }

    if (config->tickers == 0)
    {
        config->tickers = DEFAULT_TICKERS;
    }

    return true;
}

bool flash_memory_write(const device_config_t* config)
{
    if (!config) return false;

    device_config_t temp_config;
    memcpy(&temp_config, config, sizeof(device_config_t));
    temp_config.magic = FLASH_MEMORY_MAGIC;
    temp_config.crc32 = 0;
    temp_config.crc32 = crc32((const uint8_t*)&temp_config, sizeof(device_config_t));

    uint8_t buffer[FLASH_SECTOR_SIZE];
    memset(buffer, 0xFF, FLASH_SECTOR_SIZE);
    memcpy(buffer, &temp_config, sizeof(device_config_t));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    debug(INFO, "Flash config written successfully");
    return true;
}

void flash_memory_erase(void)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    debug(INFO, "Flash config erased");
}
