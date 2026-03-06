#include <ticker.h>

#include <const.h>
#include <flash_memory.h>
#include <glyphs.h>
#include <http.h>
#include <macro.h>
#include <matrix.h>
#include <menu.h>
#include <secrets_config.h>
#include <utils.h>
#include <wifi.h>
#include <currency.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/time.h>

Ticker TKR_ARR[TKR_COUNT] = {
    {XBT, TKR_XBT, 0.0f, 0.0f}, {ETH, TKR_ETH, 0.0f, 0.0f}, {SOL, TKR_SOL, 0.0f, 0.0f}};

static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_tkr_to_fetch_index = 0;
static int g_scroll_offset = 0;

static const char* currency_to_string(Currency ccy)
{
    switch (ccy)
    {
        case GBP: return CCY_GBP;
        case USD: return CCY_USD;
        case EUR: return CCY_EUR;
        case JPY: return CCY_JPY;
        case CAD: return CCY_CAD;
        case AUD: return CCY_AUD;
        case CHF: return CCY_CHF;
        case CNY: return CCY_CNY;
        case INR: return CCY_INR;
        case KRW: return CCY_KRW;
        case BRL: return CCY_BRL;
        case SGD: return CCY_SGD;
        default: return CCY_GBP;
    }
}

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

static float ticker_parse_price_from_string(const char* json, const char* key)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0.0f;

    pos += strlen(key);

    while (*pos && *pos != '"') pos++;
    if (*pos != '"') return 0.0f;
    pos++;

    return atof(pos);
}

// Display logic

static void ticker_response_callback(const char* body, size_t len, bool complete)
{
    debug(TRACE, "ticker_response_callback: len=%d, complete=%d", len, complete);

    if (!complete) return;

    if (!body || len == 0)
    {
        debug(ERROR, "Empty response for ticker index %u", g_tkr_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    debug(DEBUG, "Received ticker response: %.*s", (int)len, body);

    float current_price = ticker_parse_price_from_array(body, "\"c\":");
    float open_price = ticker_parse_price_from_string(body, "\"o\":");

    if (!current_price) current_price = 0.0f;
    if (!open_price) open_price = 0.0f;

    if (current_price > 0.0f && open_price > 0.0f)
    {
        TKR_ARR[g_tkr_to_fetch_index].current_price = current_price;
        TKR_ARR[g_tkr_to_fetch_index].open_price = open_price;
        g_data_fetched = true;

        uint64_t ms_until_next_fetch = 15 * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed current: $%.2f, open: $%.2f", current_price, open_price);
        debug(TRACE, "Next fetch scheduled in 15 minutes");
    }
    else
    {
        debug(ERROR, "Failed to parse ticker data for index %u", g_tkr_to_fetch_index);
    }
}

void ticker_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    uint64_t current_time_ms = get_current_time_ms();
    bool should_fetch = (g_next_fetch_time_ms == 0) || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        debug(INFO, "Starting ticker data fetch");

        if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, mtrx) == 0)
        {
            wifi_config_t config;
            flash_memory_read_with_defaults(&config);
            const char* currency = currency_to_string(config.currency);

            while (g_tkr_to_fetch_index < TKR_COUNT)
            {
                const char* tkr_to_fetch = TKR_ARR[g_tkr_to_fetch_index].to_str;

                char path[256];
                snprintf(path, sizeof(path), "/proxy/%s/0/public/Ticker?pair=%s%s", HOST_URL_STOCKS,
                         tkr_to_fetch, currency);

                debug(INFO, "Fetching %s via proxy: %s:%s%s", tkr_to_fetch, PROXY_HOST, PROXY_PORT,
                      path);

                uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
                int rc = http_get(PROXY_HOST, proxy_port, path, ticker_response_callback, mtrx);
                if (rc != 0)
                {
                    debug(ERROR, "HTTP request failed for %s with RC %d", tkr_to_fetch, rc);
                }

                g_tkr_to_fetch_index++;
            }

            g_tkr_to_fetch_index = 0;
            debug(INFO, "Ticker fetch complete");
        }
        else
        {
            debug(ERROR, "WiFi connection failed");
        }
    }

    if (g_data_fetched)
    {
        int total_width = 0;
        int ticker_widths[TKR_COUNT];

        for (unsigned int i = 0; i < TKR_COUNT; i++)
        {
            const Ticker* tkr = &TKR_ARR[i];
            float current = tkr->current_price;
            float open = tkr->open_price;

            char price_str[16];
            snprintf(price_str, sizeof(price_str), "%.2f", current);

            char change_perc_str[16];
            snprintf(change_perc_str, sizeof(change_perc_str), "+%.1f%%", open);

            int width = matrix_calculate_word_width_with_space(tkr->to_str) + 3 +
                        matrix_calculate_word_width_with_space(price_str) +
                        ICONS_ARR[TKR_UP].width +
                        matrix_calculate_word_width_with_space(change_perc_str) + 2;

            ticker_widths[i] = width;
            total_width += width;
        }

        matrix_clear(mtrx);

        int current_x = 0;
        for (unsigned int i = 0; i < TKR_COUNT; i++)
        {
            const Ticker* tkr = &TKR_ARR[i];

            float current = tkr->current_price;
            float open = tkr->open_price;

            float change_perc = ((current - open) / open) * 100;
            bool is_up = (change_perc >= 0.0f);

            const Glyph* icon = is_up ? &ICONS_ARR[TKR_UP] : &ICONS_ARR[TKR_DOWN];
            const RGB* colour = is_up ? &GREEN : &RED;

            char price_str[16];
            snprintf(price_str, sizeof(price_str), "%.2f", current);

            char change_perc_str[16];
            snprintf(change_perc_str, sizeof(change_perc_str), "%+.1f%%", change_perc);

            int base_x = current_x - *offset;

            for (int wrap = 0; wrap < 2; wrap++)
            {
                int tkr_x = base_x + (wrap * total_width) + 2;
                int price_x = tkr_x + matrix_calculate_word_width_with_space(tkr->to_str) + 3;
                int icon_x = price_x + matrix_calculate_word_width_with_space(price_str);
                int perc_diff_x = icon_x + icon->width + 4;

                const unsigned int y = 1;

                matrix_display_word(tkr->to_str, tkr_x, y, &DEFAULT_COLOUR);
                matrix_display_word(price_str, price_x, y, colour);
                matrix_display_icon(icon, icon_x, y, colour);
                matrix_display_word(change_perc_str, perc_diff_x, y, colour);
            }

            current_x += ticker_widths[i];
        }

        matrix_show(mtrx);

        (*offset)++;
        if (*offset >= total_width)
        {
            *offset = 0;
        }
    }
}

void ticker_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_tkr_to_fetch_index = 0;
    g_scroll_offset = 0;
}
