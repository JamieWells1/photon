#include <weather.h>

#include <const.h>
#include <glyphs.h>
#include <graphics.h>
#include <http.h>
#include <location.h>
#include <matrix.h>
#include <menu.h>
#include <secrets_config.h>
#include <wifi.h>

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/time.h>

#define PERFECT_TEMP_DISPLAY_VALUE 180
#define PERFECT_RED_TEMP 35
#define PERFECT_GREEN_TEMP 15
#define PERFECT_BLUE_TEMP -5

static float g_hourly_temps[WEATHER_HOURS];
static int g_hourly_codes[WEATHER_HOURS];
static char g_hourly_times[WEATHER_HOURS][20];
static char g_sunrise_time[20];
static char g_sunset_time[30];

static volatile int g_current_hour_index = 0;
static volatile int g_hours_fetched = 0;
static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;

// Display state cache
static float g_last_displayed_temp = -999.0f;
static int g_last_displayed_code = -1;

// Getters

int weather_current_hour_index() { return g_current_hour_index; }
int weather_hours_fetched() { return g_hours_fetched; }

// Parsing logic

static float location_get_latitude(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return LAT_LONDON;
        case LOC_MIAMI:
            return LAT_MIAMI;
        case LOC_LOS_ANGELES:
            return LAT_LOS_ANGELES;
        case LOC_CHICAGO:
            return LAT_CHICAGO;
        case LOC_TOKYO:
            return LAT_TOKYO;
        case LOC_SYDNEY:
            return LAT_SYDNEY;
        case LOC_STOCKHOLM:
            return LAT_STOCKHOLM;
        case LOC_RIO_DE_JANEIRO:
            return LAT_RIO_DE_JANEIRO;
        case LOC_CAPE_TOWN:
            return LAT_CAPE_TOWN;
        case LOC_ATHENS:
            return LAT_ATHENS;
        default:
            return 0.0;
    }
}

static float location_get_longitude(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return LON_LONDON;
        case LOC_MIAMI:
            return LON_MIAMI;
        case LOC_LOS_ANGELES:
            return LON_LOS_ANGELES;
        case LOC_CHICAGO:
            return LON_CHICAGO;
        case LOC_TOKYO:
            return LON_TOKYO;
        case LOC_SYDNEY:
            return LON_SYDNEY;
        case LOC_STOCKHOLM:
            return LON_STOCKHOLM;
        case LOC_RIO_DE_JANEIRO:
            return LON_RIO_DE_JANEIRO;
        case LOC_CAPE_TOWN:
            return LON_CAPE_TOWN;
        case LOC_ATHENS:
            return LON_ATHENS;
        default:
            return 0.0;
    }
}

static const char* location_get_timezone(Location loc)
{
    switch (loc)
    {
        case LOC_LONDON:
            return "Europe/London";
        case LOC_MIAMI:
            return "America/New_York";
        case LOC_LOS_ANGELES:
            return "America/Los_Angeles";
        case LOC_CHICAGO:
            return "America/Chicago";
        case LOC_TOKYO:
            return "Asia/Tokyo";
        case LOC_SYDNEY:
            return "Australia/Sydney";
        case LOC_STOCKHOLM:
            return "Europe/Stockholm";
        case LOC_RIO_DE_JANEIRO:
            return "America/Sao_Paulo";
        case LOC_CAPE_TOWN:
            return "Africa/Johannesburg";
        case LOC_ATHENS:
            return "Europe/Athens";
        default:
            return "UTC";
    }
}

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

static bool weather_is_daytime(int hour)
{
    if (strlen(g_sunrise_time) < 13 || strlen(g_sunset_time) < 13) return true;

    int sunrise_hour = (g_sunrise_time[11] - '0') * 10 + (g_sunrise_time[12] - '0');
    int sunset_hour = (g_sunset_time[11] - '0') * 10 + (g_sunset_time[12] - '0');

    return hour >= sunrise_hour && hour < sunset_hour;
}

static const Glyph* weather_code_to_icon(int code, int hour)
{
    bool is_day = weather_is_daytime(hour);
    if (!is_day) return &ICONS_ARR[MOON];

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

    int current_minute = 0;
    const char* current_time_pos = strstr(body, "\"current\":{\"time\":\"");
    if (current_time_pos)
    {
        current_time_pos += 20;
        char hour_str[3] = {0};
        hour_str[0] = current_time_pos[10];
        hour_str[1] = current_time_pos[11];
        g_current_hour_index = atoi(hour_str);

        char minute_str[3] = {0};
        minute_str[0] = current_time_pos[13];
        minute_str[1] = current_time_pos[14];
        current_minute = atoi(minute_str);

        debug("Current time: %d:%02d", g_current_hour_index, current_minute);
    }

    int time_count = weather_parse_time_array(body, "\"time\":[", g_hourly_times, WEATHER_HOURS);
    int temp_count =
        weather_parse_float_array(body, "\"temperature_2m\":[", g_hourly_temps, WEATHER_HOURS);
    int code_count =
        weather_parse_int_array(body, "\"weather_code\":[", g_hourly_codes, WEATHER_HOURS);

    const char* sunrise_pos = strstr(body, "\"sunrise\":[\"");
    if (sunrise_pos)
    {
        sunrise_pos += 12;
        strncpy(g_sunrise_time, sunrise_pos, 19);
        g_sunrise_time[19] = '\0';
        debug("Sunrise: %s", g_sunrise_time);
    }

    const char* sunset_pos = strstr(body, "\"sunset\":[\"");
    if (sunset_pos)
    {
        sunset_pos += 11;
        strncpy(g_sunset_time, sunset_pos, 19);
        g_sunset_time[19] = '\0';
        debug("Sunset: %s", g_sunset_time);
    }

    debug("Parsing results: time_count=%d, temp_count=%d, code_count=%d", time_count, temp_count,
          code_count);

    if (time_count > 0 && temp_count > 0 && code_count > 0)
    {
        g_hours_fetched = temp_count;
        debug("Parsed %d hours of weather data", temp_count);
        debug("Current hour (%s): temp=%.1f, code=%d", g_hourly_times[g_current_hour_index],
              g_hourly_temps[g_current_hour_index], g_hourly_codes[g_current_hour_index]);

        uint64_t minutes_until_next_hour = 60 - current_minute;
        uint64_t ms_until_next_hour = minutes_until_next_hour * 60 * 1000;

        g_data_fetched = true;
        g_next_fetch_time_ms = to_ms_since_boot(get_absolute_time()) + ms_until_next_hour;

        debug("Next fetch scheduled in %llu minutes (at next hour)", minutes_until_next_hour);
    }
}

static RGB get_dynamic_temp_display_colour(float temp)
{
    RGB col = {0, 0, 0};

    if (temp >= PERFECT_RED_TEMP)
    {
        col.r = PERFECT_TEMP_DISPLAY_VALUE;
        return col;
    }
    if (temp <= PERFECT_BLUE_TEMP)
    {
        col.b = PERFECT_TEMP_DISPLAY_VALUE;
        return col;
    }

    if (temp > PERFECT_GREEN_TEMP)
    {
        int midpoint = (PERFECT_GREEN_TEMP + PERFECT_RED_TEMP) / 2;

        if (temp <= midpoint)
        {
            float range = midpoint - PERFECT_GREEN_TEMP;
            float position = temp - PERFECT_GREEN_TEMP;
            float ratio = position / range;

            col.g = PERFECT_TEMP_DISPLAY_VALUE;
            col.r = (uint8_t)(PERFECT_TEMP_DISPLAY_VALUE * ratio);
        }
        else
        {
            float range = PERFECT_RED_TEMP - midpoint;
            float position = temp - midpoint;
            float ratio = position / range;

            col.r = PERFECT_TEMP_DISPLAY_VALUE;
            col.g = (uint8_t)(PERFECT_TEMP_DISPLAY_VALUE * (1.0f - ratio));
        }
    }
    else
    {
        int midpoint = (PERFECT_BLUE_TEMP + PERFECT_GREEN_TEMP) / 2;

        if (temp <= midpoint)
        {
            float range = midpoint - PERFECT_BLUE_TEMP;
            float position = temp - PERFECT_BLUE_TEMP;
            float ratio = position / range;

            col.b = PERFECT_TEMP_DISPLAY_VALUE;
            col.g = (uint8_t)(PERFECT_TEMP_DISPLAY_VALUE * ratio);
        }
        else
        {
            float range = PERFECT_GREEN_TEMP - midpoint;
            float position = temp - midpoint;
            float ratio = position / range;

            col.g = PERFECT_TEMP_DISPLAY_VALUE;
            col.b = (uint8_t)(PERFECT_TEMP_DISPLAY_VALUE * (1.0f - ratio));
        }
    }

    return col;
}

void weather_display(SubMode sub_mode, Matrix* mtrx, int* hour_offset_from_now_to_display)
{
    uint64_t current_time_ms = to_ms_since_boot(get_absolute_time());
    bool should_fetch = !g_data_fetched || current_time_ms >= g_next_fetch_time_ms;

    // Fetch weather data if needed
    if (should_fetch)
    {
        if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, mtrx) == 0)
        {
            float lat = location_get_latitude(LOCATION);
            float lon = location_get_longitude(LOCATION);
            const char* tz = location_get_timezone(LOCATION);

            char path[256];
            snprintf(path, sizeof(path),
                     "/v1/forecast?latitude=%.4f&longitude=%.4f"
                     "&current=temperature_2m,weather_code"
                     "&hourly=temperature_2m,weather_code"
                     "&daily=sunrise,sunset"
                     "&forecast_days=%d"
                     "&timezone=%s",
                     lat, lon, WEATHER_HOURS / 24, tz);

            http_get(HOST_URL_WEATHER, path, weather_response_callback, mtrx);
            wifi_disconnect();
        }
    }

    // Display weather data
    if (!g_data_fetched) return;

    int hour_index = g_current_hour_index;

    if (sub_mode == TEMP_HOURLY && hour_offset_from_now_to_display != NULL)
    {
        hour_index = g_current_hour_index + *hour_offset_from_now_to_display;
    }

    if (hour_index >= WEATHER_HOURS) hour_index = WEATHER_HOURS - 1;

    float current_temp = g_hourly_temps[hour_index];
    int current_code = g_hourly_codes[hour_index];

    char* time_str = g_hourly_times[hour_index];
    int hour = weather_get_hour_from_time(time_str);
    char hour_str[10];
    snprintf(hour_str, sizeof(hour_str), "%02d", hour);

    const Glyph* weather_icon = weather_code_to_icon(current_code, hour);

    if (current_temp != g_last_displayed_temp || current_code != g_last_displayed_code)
    {
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.0f°C", current_temp);

        matrix_clear(mtrx);

        int icon_starting_x = MATRIX_WIDTH - weather_icon->width - 1;
        int temp_display_starting_x =
            icon_starting_x - matrix_calculate_word_width_with_space(temp_str) - 1;

        matrix_display_word_icon_pair(hour_str, &DEFAULT_COLOUR, weather_icon, 0);

        RGB dynamic_temp = get_dynamic_temp_display_colour(current_temp);
        matrix_display_word(temp_str, temp_display_starting_x, 1, &dynamic_temp);

        debug("Displaying weather: hour='%s', temp='%s', code=%d, colour: r=%d, g=%d, b=%d",
              hour_str, temp_str, current_code, dynamic_temp.r, dynamic_temp.g, dynamic_temp.b);

        matrix_show(mtrx);

        g_last_displayed_temp = current_temp;
        g_last_displayed_code = current_code;
    }
}

void weather_cleanup(void)
{
    wifi_disconnect();
    g_last_displayed_temp = -999.0f;
    g_last_displayed_code = -1;
}
