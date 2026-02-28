#include <ticker.h>

#include <config.h>
#include <const.h>
#include <glyphs.h>
#include <http.h>
#include <macro.h>
#include <matrix.h>
#include <menu.h>
#include <secrets_config.h>
#include <utils.h>
#include <wifi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/time.h>

static volatile double g_btc_price = 0.0;
static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;

// Parsing

static float ticker_parse_price_from_array(const char* json, const char* key)
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

// Display logic

static void ticker_response_callback(const char* body, size_t len, bool complete)
{
    debug(TRACE, "ticker_response_callback: len=%d, complete=%d", len, complete);

    if (!body || len == 0) return;
    if (!complete) return;

    debug(DEBUG, "Received ticker response: %.*s", (int)len, body);

    float price = ticker_parse_price_from_array(body, "\"c\":");

    if (price > 0.0f)
    {
        g_btc_price = price;
        g_data_fetched = true;

        uint64_t ms_until_next_fetch = 15 * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed price: $%.2f", g_btc_price);
        debug(TRACE, "Next fetch scheduled in 15 minutes");
    }
}

void ticker_display(Matrix* mtrx)
{
    ASSERT_NOT_NULL(mtrx);

    uint64_t current_time_ms = get_current_time_ms();
    bool should_fetch = !g_data_fetched || current_time_ms >= g_next_fetch_time_ms;

    if (should_fetch)
    {
        if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, mtrx) == 0)
        {
            char path[256];
            snprintf(path, sizeof(path), "/0/public/Ticker?pair=%s%s", TKR_XBT, CURRENCY);

            int rc = http_get(HOST_URL_STOCKS, path, ticker_response_callback, mtrx);
            if (rc != 0)
            {
                debug(ERROR, "HTTP request failed with RC %d", rc);
            }
            wifi_disconnect();
        }
    }

    if (g_data_fetched)
    {
        char price_str[32];
        snprintf(price_str, sizeof(price_str), "$%.0f", g_btc_price);

        matrix_clear(mtrx);
        matrix_display_word_icon_pair(price_str, &DEFAULT_COLOUR, &ICONS_ARR[TICKERS], 0);
        matrix_show(mtrx);
    }
}

void ticker_cleanup(void)
{
    g_data_fetched = false;
    g_btc_price = 0.0;
    g_next_fetch_time_ms = 0;
}
