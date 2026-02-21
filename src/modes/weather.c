#include <weather.h>

#include <displays.h>
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

#define WEATHER_HOURS 24
#define WEATHER_REFRESH_INTERVAL_MS (60 * 60 * 1000)

static float hourly_temps[WEATHER_HOURS];
static int hourly_codes[WEATHER_HOURS];
static bool data_fetched = false;
static uint64_t last_fetch_time_ms = 0;
static bool fetch_in_progress = false;

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

static IconType weather_code_to_icon(int code)
{
    if (code == 0 || code == 1)
    {
        return SUNNY;
    }
    else if (code == 2 || code == 3)
    {
        return SUNNY_CLOUDY;
    }
    else if (code >= 51 && code <= 67)
    {
        return RAIN;
    }
    else if (code >= 71 && code <= 77)
    {
        return SNOW;
    }
    else if (code >= 95)
    {
        return LIGHTNING;
    }
    else
    {
        return CLOUDY;
    }
}

static void weather_response_callback(const char* body, size_t len, bool complete)
{
    if (!body || len == 0) return;

    int temp_count =
        weather_parse_float_array(body, "\"temperature_2m\":[", hourly_temps, WEATHER_HOURS);
    int code_count =
        weather_parse_int_array(body, "\"weather_code\":[", hourly_codes, WEATHER_HOURS);

    if (temp_count > 0 && code_count > 0)
    {
        debug("Parsed %d hours of weather data", temp_count);
        data_fetched = true;
        last_fetch_time_ms = to_ms_since_boot(get_absolute_time());
        fetch_in_progress = false;

        debug("Weather data received, disconnecting WiFi to save power");
        wifi_disconnect();
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
             "&hourly=temperature_2m,weather_code"
             "&forecast_days=1"
             "&timezone=Europe/London",
             LOCATION_LAT, LOCATION_LON);

    const char* host = "api.open-meteo.com";
    http_get(host, path, weather_response_callback);
}

void weather_display(SubMenu sub_mode, Matrix* mtrx)
{
    uint64_t current_time_ms = to_ms_since_boot(get_absolute_time());
    uint64_t time_since_last_fetch = current_time_ms - last_fetch_time_ms;

    bool should_fetch = (!data_fetched || time_since_last_fetch >= WEATHER_REFRESH_INTERVAL_MS) &&
                        !fetch_in_progress;

    // Get current WiFi state
    WifiState wifi_state = wifi_get_state();

    if (should_fetch && wifi_state == WIFI_DISCONNECTED)
    {
        debug("Starting WiFi connection...");
        fetch_in_progress = true;
        wifi_join_async(WIFI_SSID, WIFI_PASSWORD);
    }

    if (wifi_state == WIFI_CONNECTING)
    {
        wifi_join_async(WIFI_SSID, WIFI_PASSWORD);

        matrix_clear(mtrx);
        matrix_display_word_icon_pair("WIFI", &DEFAULT_COLOUR, &ICONS_ARR[WEATHER], 0);
        matrix_show(mtrx);
        return;
    }

    if (should_fetch && wifi_state == WIFI_CONNECTED && fetch_in_progress)
    {
        debug("WiFi connected, fetching weather...");
        weather_fetch_data();

        matrix_clear(mtrx);
        matrix_display_word_icon_pair("LOAD", &DEFAULT_COLOUR, &ICONS_ARR[WEATHER], 0);
        matrix_show(mtrx);
        return;
    }

    if (wifi_state == WIFI_FAILED)
    {
        fetch_in_progress = false;
        matrix_clear(mtrx);
        matrix_display_word_icon_pair("ERROR", &RED, NULL, 0);
        matrix_show(mtrx);
        return;
    }

    if (!data_fetched)
    {
        debug("Waiting for weather data...");
        return;
    }

    int hour_index = 0;

    if (sub_mode.mode == TEMP_CURRENT)
    {
        hour_index = 0;
    }
    else if (sub_mode.mode == TEMP_HOURLY)
    {
        hour_index = 1;
    }

    if (hour_index >= WEATHER_HOURS) hour_index = WEATHER_HOURS - 1;

    IconType icon = weather_code_to_icon(hourly_codes[hour_index]);

    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.0fC", hourly_temps[hour_index]);

    matrix_display_word_icon_pair(temp_str, &DEFAULT_COLOUR, &ICONS_ARR[icon], 0);
}
