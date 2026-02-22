#include <stddef.h>
#include <weather.h>

#include <const.h>
#include <glyphs.h>
#include <graphics.h>
#include <http.h>
#include <matrix.h>
#include <menu.h>
#include <secrets.h>
#include <wifi.h>

#include <pico/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

#define WEATHER_HOURS 48
#define WEATHER_REFRESH_INTERVAL_MS (60 * 60 * 1000)

static float g_hourly_temps[WEATHER_HOURS];
static int g_hourly_codes[WEATHER_HOURS];
static char g_hourly_times[WEATHER_HOURS][20];

static volatile int g_current_hour_index = 0;
static volatile bool g_data_fetched = false;
static volatile uint64_t g_last_fetch_time_ms = 0;
static bool g_http_request_sent = false;
static volatile uint64_t g_data_ready_time_ms = 0;

// WiFi connection manager
static WifiConnectionManager g_wifi_manager;
static int g_connection_dots_counter = 0;

// Display state cache
static WifiState g_last_wifi_state = WIFI_DISCONNECTED;
static bool g_last_data_fetched = false;
static float g_last_displayed_temp = -999.0f;
static int g_last_displayed_code = -1;

#define DATA_DISPLAY_DELAY_MS 500

static int weather_parse_float_array(const char* json, const char* key, float* values,
                                     int max_values)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0;

    pos += strlen(key);

    int count = 0;
    while (count < max_values && *pos != ']')
    {
        while (*pos == ' ' || *pos == ',') pos++;

        if (*pos == ']') break;

        values[count] = atof(pos);
        count++;

        while (*pos != ',' && *pos != ']' && *pos != '\0') pos++;
    }

    return count;
}

static int weather_parse_int_array(const char* json, const char* key, int* values, int max_values)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0;

    pos += strlen(key);

    int count = 0;
    while (count < max_values && *pos != ']')
    {
        while (*pos == ' ' || *pos == ',') pos++;

        if (*pos == ']') break;

        values[count] = atoi(pos);
        count++;

        while (*pos != ',' && *pos != ']' && *pos != '\0') pos++;
    }

    return count;
}

static int weather_parse_time_array(const char* json, const char* key, char times[][20],
                                    int max_values)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0;

    pos += strlen(key);

    int count = 0;
    while (count < max_values && *pos != ']')
    {
        while (*pos == ' ' || *pos == ',') pos++;

        if (*pos == ']') break;
        if (*pos == '"')
        {
            pos++;
            int i = 0;
            while (*pos != '"' && *pos != '\0' && i < 19)
            {
                times[count][i++] = *pos++;
            }
            times[count][i] = '\0';
            count++;

            while (*pos != ',' && *pos != ']' && *pos != '\0') pos++;
        }
        else
        {
            break;
        }
    }

    return count;
}

static int weather_get_hour_from_time(const char* time_str)
{
    if (!time_str || strlen(time_str) < 13) return 0;

    char hour_str[3] = {0};
    hour_str[0] = time_str[11];
    hour_str[1] = time_str[12];
    return atoi(hour_str);
}

static const Glyph* weather_code_to_icon(int code)
{
    if (code == 0 || code == 1)
    {
        return &ICONS_ARR[SUNNY];
    }
    else if (code == 2 || code == 3)
    {
        return &ICONS_ARR[SUNNY_CLOUDY];
    }
    else if (code >= 51 && code <= 67)
    {
        return &ICONS_ARR[RAIN];
    }
    else if (code >= 71 && code <= 77)
    {
        return &ICONS_ARR[SNOW];
    }
    else if (code >= 95)
    {
        return &ICONS_ARR[LIGHTNING];
    }
    else
    {
        return &ICONS_ARR[CLOUDY];
    }
}

static void weather_response_callback(const char* body, size_t len, bool complete)
{
    debug("weather_response_callback: len=%d, complete=%d", len, complete);

    if (!body || len == 0) return;

    if (!complete) return;

    const char* current_time_pos = strstr(body, "\"current\":{\"time\":\"");
    if (current_time_pos)
    {
        current_time_pos += 20;
        char hour_str[3] = {0};
        hour_str[0] = current_time_pos[10];
        hour_str[1] = current_time_pos[11];
        g_current_hour_index = atoi(hour_str);
        debug("Current hour index: %d", g_current_hour_index);
    }

    int time_count = weather_parse_time_array(body, "\"time\":[", g_hourly_times, WEATHER_HOURS);
    int temp_count =
        weather_parse_float_array(body, "\"temperature_2m\":[", g_hourly_temps, WEATHER_HOURS);
    int code_count =
        weather_parse_int_array(body, "\"weather_code\":[", g_hourly_codes, WEATHER_HOURS);

    debug("Parsing results: time_count=%d, temp_count=%d, code_count=%d", time_count, temp_count,
          code_count);

    if (time_count > 0 && temp_count > 0 && code_count > 0)
    {
        debug("Parsed %d hours of weather data", temp_count);
        debug("Current hour (%s): temp=%.1f, code=%d", g_hourly_times[g_current_hour_index],
              g_hourly_temps[g_current_hour_index], g_hourly_codes[g_current_hour_index]);
        g_data_fetched = true;
        g_last_fetch_time_ms = to_ms_since_boot(get_absolute_time());
        g_data_ready_time_ms = to_ms_since_boot(get_absolute_time());

        // Reset WiFi manager now that data fetch is complete
        wifi_manager_reset(&g_wifi_manager);

        debug("Weather data received (WiFi will be disconnected by main thread)");
    }
}

static void weather_fetch_data(void)
{
    if (wifi_join(WIFI_SSID, WIFI_PASSWORD) != 0)
    {
        return;
    }

    debug("Fetching weather data...");

    char path[256];
    snprintf(path, sizeof(path),
             "/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,weather_code"
             "&hourly=temperature_2m,weather_code"
             "&forecast_days=%.0d"
             "&timezone=Europe/London",
             LOCATION_LAT, LOCATION_LON, WEATHER_HOURS / 24);

    const char* host = "api.open-meteo.com";
    http_get(host, path, weather_response_callback);
}

static void weather_animate_status_dots(char* status_text, size_t max_size)
{
    wifi_append_connecting_dots(status_text, max_size, &g_connection_dots_counter);
}

void weather_display(SubMenu sub_mode, Matrix* mtrx)
{
    uint64_t current_time_ms = to_ms_since_boot(get_absolute_time());
    uint64_t time_since_last_fetch = current_time_ms - g_last_fetch_time_ms;

    bool should_fetch = !g_data_fetched || time_since_last_fetch >= WEATHER_REFRESH_INTERVAL_MS;

    if (should_fetch && !wifi_manager_is_connecting(&g_wifi_manager) &&
        !wifi_manager_has_failed(&g_wifi_manager) && wifi_get_state() == WIFI_DISCONNECTED)
    {
        debug("Starting WiFi connection for weather data...");
        wifi_manager_init(&g_wifi_manager, MAX_WIFI_CONNECTION_ATTEMPTS);
        wifi_manager_start(&g_wifi_manager, WIFI_SSID, WIFI_PASSWORD);
        g_http_request_sent = false;
    }
    WifiState wifi_state = wifi_manager_update(&g_wifi_manager, WIFI_SSID, WIFI_PASSWORD);

    if (wifi_state == WIFI_CONNECTING)
    {
        char current_display[256] = "WIFI";
        weather_animate_status_dots(current_display, sizeof(current_display));

        matrix_clear(mtrx);
        matrix_display_word_icon_pair(current_display, &DEFAULT_COLOUR, &ICONS_ARR[WIFI], 0);
        matrix_show(mtrx);
        g_last_wifi_state = wifi_state;
        return;
    }

    // Handle WiFi connection successful - fetch data
    if (should_fetch && wifi_state == WIFI_CONNECTED && !g_http_request_sent)
    {
        debug("WiFi connected, fetching weather data...");
        weather_fetch_data();
        g_http_request_sent = true;

        char current_display[256] = "DATA";
        weather_animate_status_dots(current_display, sizeof(current_display));

        matrix_clear(mtrx);
        matrix_display_word_icon_pair(current_display, &DEFAULT_COLOUR, &ICONS_ARR[DATA], 0);
        matrix_show(mtrx);
        g_last_wifi_state = wifi_state;
        return;
    }

    // Still waiting for data after HTTP request sent
    if (wifi_state == WIFI_CONNECTED && g_http_request_sent && !g_data_fetched)
    {
        char current_display[256] = "DATA";
        weather_animate_status_dots(current_display, sizeof(current_display));

        matrix_clear(mtrx);
        matrix_display_word_icon_pair(current_display, &DEFAULT_COLOUR, &ICONS_ARR[DATA], 0);
        matrix_show(mtrx);
        g_last_wifi_state = wifi_state;
        return;
    }

    if (wifi_state == WIFI_FAILED || wifi_manager_has_failed(&g_wifi_manager))
    {
        if (g_last_wifi_state != WIFI_FAILED)
        {
            matrix_clear(mtrx);
            matrix_display_word_icon_pair("TIMEOUT", &RED, NULL, 0);
            matrix_show(mtrx);
            g_last_wifi_state = WIFI_FAILED;
            g_last_displayed_temp = -999.0f;
        }
        return;
    }

    // Fallback status if no data yet
    if (!g_data_fetched)
    {
        if (wifi_manager_has_failed(&g_wifi_manager))
        {
            debug("Showing connection failed status");

            if (g_last_wifi_state != WIFI_FAILED || !g_last_data_fetched)
            {
                matrix_clear(mtrx);
                matrix_display_word_icon_pair("TIMEOUT", &RED, NULL, 0);
                matrix_show(mtrx);
                g_last_wifi_state = WIFI_FAILED;
                g_last_data_fetched = false;
            }
            return;
        }

        debug("Displaying fallback status (no data fetched yet)");
        char current_display[256];

        if (wifi_manager_is_connecting(&g_wifi_manager) && wifi_state == WIFI_CONNECTED)
        {
            debug("Showing DATA status");
            strcpy(current_display, "DATA");
        }
        else
        {
            debug("Showing WIFI status");
            strcpy(current_display, "WIFI");
        }

        weather_animate_status_dots(current_display, sizeof(current_display));

        matrix_clear(mtrx);
        matrix_display_word_icon_pair(current_display, &DEFAULT_COLOUR, &ICONS_ARR[DATA], 0);
        matrix_show(mtrx);
        g_last_wifi_state = wifi_state;
        g_last_data_fetched = false;
        return;
    }

    if (g_data_ready_time_ms > 0 &&
        (current_time_ms - g_data_ready_time_ms) < DATA_DISPLAY_DELAY_MS)
    {
        char current_display[256] = "DATA";
        weather_animate_status_dots(current_display, sizeof(current_display));

        matrix_clear(mtrx);
        matrix_display_word_icon_pair(current_display, &DEFAULT_COLOUR, &ICONS_ARR[DATA], 0);
        matrix_show(mtrx);
        g_last_wifi_state = wifi_state;
        return;
    }

    if (wifi_state == WIFI_CONNECTED)
    {
        debug("Disconnecting WiFi to save power");
        wifi_disconnect();
    }

    int hour_index = g_current_hour_index;

    if (sub_mode.mode == TEMP_CURRENT)
    {
        hour_index = g_current_hour_index;
    }
    else if (sub_mode.mode == TEMP_HOURLY)
    {
        hour_index = g_current_hour_index + 1;
    }

    if (hour_index >= WEATHER_HOURS) hour_index = WEATHER_HOURS - 1;

    float current_temp = g_hourly_temps[hour_index];
    int current_code = g_hourly_codes[hour_index];

    char* time_str = g_hourly_times[hour_index];
    char hour_str[10];
    snprintf(hour_str, sizeof(hour_str), "%02d", weather_get_hour_from_time(time_str));

    const Glyph* weather_icon = weather_code_to_icon(current_code);

    if (current_temp != g_last_displayed_temp || current_code != g_last_displayed_code ||
        !g_last_data_fetched)
    {
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.0f°C", current_temp);

        debug("Displaying weather: hour='%s', temp='%s', code=%d", hour_str, temp_str,
              current_code);

        matrix_clear(mtrx);

        int icon_starting_x = MATRIX_WIDTH - weather_icon->width - 1;
        int temp_display_starting_x =
            icon_starting_x - matrix_calculate_word_width_with_space(temp_str) - 1;

        matrix_display_word_icon_pair(hour_str, &DEFAULT_COLOUR, weather_icon, 0);
        matrix_display_word(temp_str, temp_display_starting_x, 1, &DEFAULT_COLOUR);
        matrix_show(mtrx);

        g_last_displayed_temp = current_temp;
        g_last_displayed_code = current_code;
        g_last_data_fetched = true;
    }

    g_last_wifi_state = wifi_state;
}

void weather_cleanup(void)
{
    if (wifi_manager_is_connecting(&g_wifi_manager))
    {
        wifi_disconnect();
    }

    wifi_manager_reset(&g_wifi_manager);

    g_last_wifi_state = WIFI_DISCONNECTED;
    g_last_data_fetched = false;
    g_last_displayed_temp = -999.0f;
    g_last_displayed_code = -1;
    g_connection_dots_counter = 0;
}
