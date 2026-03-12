/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <update.h>

#include <const.h>
#include <flash_memory.h>
#include <http.h>
#include <macro.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <pico/bootrom.h>
#include <pico/stdlib.h>

#define UPDATE_VERSION_PATH "/firmware/version.txt"
#define UPDATE_SERVER_PORT 8090

static bool g_update_available = false;
static char g_remote_version[9] = {0};
static char g_version_response[128] = {0};
static size_t g_version_response_len = 0;

static void version_check_callback(const char* body, size_t len, bool complete)
{
    if (g_version_response_len + len < sizeof(g_version_response) - 1)
    {
        memcpy(g_version_response + g_version_response_len, body, len);
        g_version_response_len += len;
        g_version_response[g_version_response_len] = '\0';
    }

    if (complete)
    {
        const char* start = g_version_response;
        while (*start && !isdigit((unsigned char)*start))
        {
            start++;
        }

        if (strlen(start) >= 8)
        {
            strncpy(g_remote_version, start, 8);
            g_remote_version[8] = '\0';
            debug(INFO, "Remote version: %s", g_remote_version);
        }
        else
        {
            debug(WARN, "Invalid version format from server: %s", g_version_response);
        }
    }
}

ReturnCode update_check_version(Matrix* mtrx)
{
    device_config_t config;
    if (!flash_memory_read(&config))
    {
        debug(WARN, "No flash config, using BUILD_VERSION");
        strncpy(config.firmware_version, BUILD_VERSION, sizeof(config.firmware_version) - 1);
        config.firmware_version[sizeof(config.firmware_version) - 1] = '\0';
    }

    debug(INFO, "Current firmware version: %s", config.firmware_version);

    g_update_available = false;
    g_remote_version[0] = '\0';
    g_version_response[0] = '\0';
    g_version_response_len = 0;

    ReturnCode rc =
        http_get(PROXY_HOST, UPDATE_SERVER_PORT, UPDATE_VERSION_PATH, version_check_callback, mtrx, true);

    if (RC_FAILED(rc))
    {
        debug(WARN, "Version check failed: %s", rc_to_string(rc));
        return rc;
    }

    if (strlen(g_remote_version) == 8)
    {
        int comparison = strcmp(g_remote_version, config.firmware_version);
        if (comparison > 0)
        {
            g_update_available = true;
            debug(INFO, "Update available: %s > %s", g_remote_version, config.firmware_version);
        }
        else
        {
            debug(INFO, "Firmware is up to date");
        }
    }
    else
    {
        debug(WARN, "Invalid remote version, skipping update check");
    }

    return RC_OK;
}

bool update_is_available(void) { return g_update_available; }

const char* update_get_remote_version(void) { return g_remote_version; }

void update_enter_bootloader_mode(void)
{
    debug(INFO, "Entering bootloader mode for WebUSB flashing");
    sleep_ms(100);
    reset_usb_boot(0, 0);
}
