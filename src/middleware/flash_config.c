#include <flash_config.h>

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

bool flash_config_exists(void)
{
    const uint8_t* flash_ptr = (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
    wifi_config_t config;
    memcpy(&config, flash_ptr, sizeof(wifi_config_t));

    if (config.magic != FLASH_CONFIG_MAGIC)
    {
        return false;
    }

    uint32_t stored_crc = config.crc32;
    config.crc32 = 0;
    uint32_t calculated_crc = crc32((const uint8_t*)&config, sizeof(wifi_config_t));

    return stored_crc == calculated_crc;
}

bool flash_config_read(wifi_config_t* config)
{
    if (!config) return false;

    const uint8_t* flash_ptr = (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(config, flash_ptr, sizeof(wifi_config_t));

    if (config->magic != FLASH_CONFIG_MAGIC)
    {
        debug(WARN, "Flash config magic mismatch");
        return false;
    }

    uint32_t stored_crc = config->crc32;
    config->crc32 = 0;
    uint32_t calculated_crc = crc32((const uint8_t*)config, sizeof(wifi_config_t));
    config->crc32 = stored_crc;

    if (stored_crc != calculated_crc)
    {
        debug(WARN, "Flash config CRC mismatch");
        return false;
    }

    debug(INFO, "Flash config read successfully");
    return true;
}

bool flash_config_write(const wifi_config_t* config)
{
    if (!config) return false;

    wifi_config_t temp_config;
    memcpy(&temp_config, config, sizeof(wifi_config_t));
    temp_config.magic = FLASH_CONFIG_MAGIC;
    temp_config.crc32 = 0;
    temp_config.crc32 = crc32((const uint8_t*)&temp_config, sizeof(wifi_config_t));

    uint8_t buffer[FLASH_SECTOR_SIZE];
    memset(buffer, 0xFF, FLASH_SECTOR_SIZE);
    memcpy(buffer, &temp_config, sizeof(wifi_config_t));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    debug(INFO, "Flash config written successfully");
    return true;
}

void flash_config_erase(void)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    debug(INFO, "Flash config erased");
}
