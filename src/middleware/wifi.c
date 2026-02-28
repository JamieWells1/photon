#include <wifi.h>

#include <const.h>
#include <glyphs.h>
#include <graphics.h>
#include <macro.h>

#include <stdio.h>
#include <string.h>

#include <pico/cyw43_arch.h>
#include <pico/time.h>

static bool initialized = false;
static bool connected = false;

static int wifi_init(void)
{
    if (initialized) return 0;

    if (cyw43_arch_init())
    {
        debug("Wi-Fi init failed");
        return -1;
    }

    debug("Wi-Fi initialized");
    initialized = true;
    return 0;
}

static void wifi_enable_sta_mode(void)
{
    cyw43_arch_enable_sta_mode();
    debug("Wi-Fi station mode enabled");
}

static void wifi_display_status(Matrix* mtrx, int dot_counter)
{
    char status[256] = "WIFI";

    // Append animated dots
    size_t len = strlen(status);
    int num_dots = (dot_counter % 3) + 1;
    for (int i = 0; i < num_dots && len + i + 1 < sizeof(status); i++)
    {
        status[len + i] = '.';
        status[len + i + 1] = '\0';
    }

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(status, &DEFAULT_COLOUR, &ICONS_ARR[WIFI], 0);
    matrix_show(mtrx);
}

int wifi_connect(const char* ssid, const char* password, Matrix* mtrx)
{
    if (connected)
    {
        debug("Already connected to WiFi");
        return 0;
    }

    if (!initialized)
    {
        if (wifi_init() != 0) return -1;
        wifi_enable_sta_mode();
    }

    debug("Starting WiFi connection...");

    // Start async connection
    cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    int dot_counter = 0;
    int attempts = 0;
    int display_frame_counter = 0;

    // Block until connected or timeout
    while (attempts < MAX_WIFI_CONNECTION_ATTEMPTS)
    {
        // Update display every 3-4 polls (300-400ms between frames)
        if (display_frame_counter % 4 == 0)
        {
            wifi_display_status(mtrx, dot_counter);
            dot_counter++;
        }
        display_frame_counter++;

        // Check connection status
        int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

        if (link_status == CYW43_LINK_UP)
        {
            debug("WiFi connected successfully");
            connected = true;
            return 0;
        }
        else if (link_status == CYW43_LINK_FAIL ||
                 link_status == CYW43_LINK_BADAUTH ||
                 link_status == CYW43_LINK_NONET)
        {
            debug("WiFi connection failed (status: %d)", link_status);
            connected = false;

            // Display timeout
            matrix_clear(mtrx);
            matrix_display_word_icon_pair("TIMEOUT", &RED, NULL, 0);
            matrix_show(mtrx);
            sleep_ms(1000);

            return -1;
        }

        attempts++;
        sleep_ms(100);  // Poll every 100ms
    }

    debug("WiFi connection timeout");
    connected = false;

    // Display timeout
    matrix_clear(mtrx);
    matrix_display_word_icon_pair("TIMEOUT", &RED, NULL, 0);
    matrix_show(mtrx);
    sleep_ms(1000);

    return -1;
}

void wifi_disconnect(void)
{
    if (!connected)
    {
        debug("WiFi already disconnected");
        return;
    }

    debug("Disconnecting WiFi to save power...");
    cyw43_arch_deinit();
    connected = false;
    initialized = false;
    debug("WiFi disconnected");
}
