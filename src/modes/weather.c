#include <weather.h>

#include <displays.h>
#include <graphics.h>
#include <http.h>
#include <matrix.h>
#include <menu.h>
#include <secrets.h>
#include <wifi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEATHER_HOURS 24

static float hourly_temps[WEATHER_HOURS];
static int hourly_codes[WEATHER_HOURS];
static bool data_fetched = false;

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
        debug("Parsed %d hours of weather data\n", temp_count);
        debug("Hour 0 (now): %.1f°C, code: %d\n", hourly_temps[0], hourly_codes[0]);
        debug("Hour 1: %.1f°C, code: %d\n", hourly_temps[1], hourly_codes[1]);
        debug("Hour 23: %.1f°C, code: %d\n", hourly_temps[23], hourly_codes[23]);
        data_fetched = true;
    }
}

static void weather_fetch_data(void)
{
    if (wifi_join(WIFI_SSID, WIFI_PASSWORD) != 0)
    {
        return;
    }

    debug("Fetching weather data...\n");

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

void weather_display(SubMenu sub_mode)
{
    static bool initialized = false;
    if (!initialized)
    {
        weather_fetch_data();
        initialized = true;
    }

    if (!data_fetched)
    {
        debug("Waiting for weather data...\n");
        return;
    }

    // Determine which hour to display based on sub_mode
    int hour_index = 0;  // Default to current hour

    if (sub_mode.mode == TEMP_CURRENT)
    {
        hour_index = 0;  // Current hour
    }
    else if (sub_mode.mode == TEMP_HOURLY)
    {
        // For now, just show next hour
        // You could cycle through hours with rotary encoder input
        hour_index = 1;
    }

    // Bounds check
    if (hour_index >= WEATHER_HOURS) hour_index = WEATHER_HOURS - 1;

    IconType icon = weather_code_to_icon(hourly_codes[hour_index]);

    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.0fC", hourly_temps[hour_index]);

    debug("Display hour %d: %s, icon: %d\n", hour_index, temp_str, icon);

    // TODO: Display on matrix
    // matrix_display_word_icon_pair(temp_str, &DEFAULT_COLOUR, &ICONS_ARR[icon], 0);
}
