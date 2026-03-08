#include <utils.h>

#include <stdlib.h>
#include <string.h>

uint64_t get_current_time_ms(void) { return to_ms_since_boot(get_absolute_time()); }

float parse_price_from_json_array(const char* json, const char* key)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0.0f;

    pos += strlen(key);

    while (*pos && *pos != '[') pos++;
    if (*pos != '[') return 0.0f;
    pos++;

    while (*pos == ' ' || *pos == '"') pos++;

    return atof(pos);
}

float parse_price_from_json_string(const char* json, const char* key)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0.0f;

    pos += strlen(key);

    while (*pos && *pos != '"') pos++;
    if (*pos != '"') return 0.0f;
    pos++;

    return atof(pos);
}
