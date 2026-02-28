#include <ticker.h>

#include <const.h>
#include <glyphs.h>
#include <http.h>
#include <macro.h>
#include <matrix.h>
#include <menu.h>
#include <secrets_config.h>
#include <wifi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile double g_btc_price = 0.0;
static volatile bool g_data_fetched = false;

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
        debug(INFO, "BTC price: $%.2f", g_btc_price);
    }
}

void ticker_display(Matrix* mtrx)
{
    if (!g_data_fetched)
    {
        if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, mtrx) == 0)
        {
            int rc = http_get(HOST_URL_STOCKS, "/0/public/Ticker?pair=XBTUSD",
                              ticker_response_callback, mtrx);
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
void ticker_cleanup()
{
    g_data_fetched = false;
    g_btc_price = 0.0;
}
