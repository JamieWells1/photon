#include <config.h>

#include <const.h>
#include <error.h>
#include <flash_memory.h>
#include <glyphs.h>
#include <http_server.h>
#include <macro.h>
#include <matrix.h>
#include <wifi.h>

#include <stdio.h>

#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>

static bool server_started = false;
static int scroll_offset = 0;

void config_display(Matrix* mtrx)
{
    if (http_server_config_saved())
    {
        debug(INFO, "Config saved, rebooting...");

        matrix_clear(mtrx);
        matrix_display_word_icon_pair("REBOOT...", &ORANGE, NULL, 0);
        matrix_show(mtrx);

        sleep_ms(2000);

        watchdog_enable(1, 1);
        while (1);
    }

    if (!server_started)
    {
        if (!wifi_is_connected())
        {
            debug(INFO, "WiFi not connected, reconnecting for CONFIG mode...");

            device_config_t config;
            if (!flash_memory_read(&config))
            {
                debug(ERROR, "Failed to read WiFi config from flash");
                matrix_clear(mtrx);
                matrix_display_word_icon_pair("NO WIFI", &RED, &ICONS_ARR[WIFI], 0);
                matrix_show(mtrx);
                return;
            }

            ReturnCode rc = wifi_connect(config.ssid, config.password, mtrx);
            if (RC_FAILED(rc))
            {
                debug(ERROR, "Failed to reconnect WiFi for CONFIG mode: %s", rc_to_string(rc));
                matrix_clear(mtrx);
                matrix_display_word_icon_pair("NO WIFI", &RED, &ICONS_ARR[WIFI], 0);
                matrix_show(mtrx);
                return;
            }
        }

        wifi_start_mdns();

        http_server_start();
        server_started = true;
        debug(INFO, "======================================");
        debug(INFO, "CONFIG MODE ACTIVE");
        debug(INFO, "Web interface available at:");
        debug(INFO, "  http://photon.local");
        debug(INFO, "  (or use IP address shown above)");
        debug(INFO, "======================================");
    }

    const char* url_text = "    http://photon.local";
    int total_width = matrix_calculate_word_width_with_space(url_text) + 4;

    matrix_clear(mtrx);

    for (int wrap = 0; wrap < 2; wrap++)
    {
        int text_x = (wrap * total_width) - scroll_offset;
        int text_y = 1;
        matrix_display_word(url_text, text_x, text_y, &BLUE);
    }

    matrix_show(mtrx);

    scroll_offset++;
    if (scroll_offset >= total_width)
    {
        scroll_offset = 0;
    }

    cyw43_arch_poll();
}

void config_cleanup(void)
{
    if (server_started)
    {
        http_server_stop();
        debug(INFO, "CONFIG MODE EXITED - Web interface no longer accessible");
    }
    server_started = false;
    scroll_offset = 0;
}
