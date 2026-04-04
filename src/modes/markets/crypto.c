/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <crypto.h>

#include <const.h>
#include <currency.h>
#include <error.h>
#include <flash_memory.h>
#include <glyphs.h>
#include <http.h>
#include <macro.h>
#include <matrix.h>
#include <utils.h>
#include <wifi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/time.h>

Crypto CRYPTO_ARR[CRYPTO_COUNT] = {
    {XBT, CRYPTO_XBT, 0.0f, 0.0f},   {ETH, CRYPTO_ETH, 0.0f, 0.0f},
    {SOL, CRYPTO_SOL, 0.0f, 0.0f},   {ADA, CRYPTO_ADA, 0.0f, 0.0f},
    {DOT, CRYPTO_DOT, 0.0f, 0.0f},   {LINK, CRYPTO_LINK, 0.0f, 0.0f},
    {AVAX, CRYPTO_AVAX, 0.0f, 0.0f}, {DOGE, CRYPTO_DOGE, 0.0f, 0.0f},
    {XRP, CRYPTO_XRP, 0.0f, 0.0f},   {LTC, CRYPTO_LTC, 0.0f, 0.0f},
};

static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_crypto_to_fetch_index = 0;
static int g_scroll_offset = 0;
static int g_url_scroll_offset = 0;
static uint16_t g_enabled_mask = DEFAULT_CRYPTO;

static const char* currency_to_string(Currency ccy)
{
    switch (ccy)
    {
        case GBP:
            return CCY_GBP;
        case USD:
            return CCY_USD;
        case EUR:
            return CCY_EUR;
        case JPY:
            return CCY_JPY;
        case CAD:
            return CCY_CAD;
        case AUD:
            return CCY_AUD;
        case CHF:
            return CCY_CHF;
        case CNY:
            return CCY_CNY;
        case INR:
            return CCY_INR;
        case KRW:
            return CCY_KRW;
        case BRL:
            return CCY_BRL;
        case SGD:
            return CCY_SGD;
        default:
            return CCY_GBP;
    }
}

bool crypto_is_enabled(CryptoIndex idx) { return (g_enabled_mask & (1 << idx)) != 0; }

void crypto_set_enabled_mask(uint16_t mask)
{
    if (mask == 0) mask = DEFAULT_CRYPTO;
    g_enabled_mask = mask;
}

// Parsing

static float crypto_parse_price_from_array(const char* json, const char* key)
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

static float crypto_parse_price_from_string(const char* json, const char* key)
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

static void crypto_response_callback(const char* body, size_t len, bool complete)
{
    debug(TRACE, "crypto_response_callback: len=%d, complete=%d", len, complete);

    if (!complete) return;

    if (!body || len == 0)
    {
        debug(ERROR, "Empty response for crypto index %u", g_crypto_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    debug(DEBUG, "Received crypto response: %.*s", (int)len, body);

    float current_price = crypto_parse_price_from_array(body, "\"c\":");
    float open_price = crypto_parse_price_from_string(body, "\"o\":");

    if (!current_price) current_price = 0.0f;
    if (!open_price) open_price = 0.0f;

    if (current_price > 0.0f && open_price > 0.0f)
    {
        CRYPTO_ARR[g_crypto_to_fetch_index].current_price = current_price;
        CRYPTO_ARR[g_crypto_to_fetch_index].open_price = open_price;
        g_data_fetched = true;

        uint64_t ms_until_next_fetch = TKR_FETCH_INTERVAL_MINUTES * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed current: $%.2f, open: $%.2f", current_price, open_price);
        debug(TRACE, "Next fetch scheduled in 15 minutes");
    }
    else
    {
        debug(ERROR, "Failed to parse crypto data for index %u", g_crypto_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
}

static void crypto_display_url(Matrix* mtrx)
{
    const char* url_text = "    http://delta.local/watchlist";
    int total_width = matrix_calculate_word_width_with_space(url_text) + 4;

    matrix_clear(mtrx);

    for (int wrap = 0; wrap < 2; wrap++)
    {
        int text_x = (wrap * total_width) - g_url_scroll_offset;
        int text_y = 1;
        matrix_display_word(url_text, text_x, text_y, &BLUE);
    }

    matrix_show(mtrx);
}

void crypto_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    // Check if any crypto currencies are enabled
    uint8_t enabled_count = 0;
    for (unsigned int i = 0; i < CRYPTO_COUNT; i++)
    {
        if (crypto_is_enabled(i)) enabled_count++;
    }

    if (enabled_count == 0)
    {
        crypto_display_url(mtrx);
        g_url_scroll_offset++;
        int url_width =
            matrix_calculate_word_width_with_space("    http://delta.local/watchlist") + 4;
        if (g_url_scroll_offset >= url_width)
        {
            g_url_scroll_offset = 0;
        }
        return;
    }

    uint64_t current_time_ms = get_current_time_ms();
    bool is_first_fetch = (g_next_fetch_time_ms == 0);
    bool should_fetch = is_first_fetch || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        debug(INFO, "Starting crypto data fetch");

        if (wifi_is_connected())
        {
            device_config_t config;
            flash_memory_read(&config);
            crypto_set_enabled_mask(config.crypto);
            const char* currency = currency_to_string(config.currency);

            while (g_crypto_to_fetch_index < CRYPTO_COUNT)
            {
                if (!crypto_is_enabled(g_crypto_to_fetch_index))
                {
                    g_crypto_to_fetch_index++;
                    continue;
                }

                const char* crypto_to_fetch = CRYPTO_ARR[g_crypto_to_fetch_index].to_str;

                char path[256];
                snprintf(path, sizeof(path), "/proxy/%s/0/public/Ticker?pair=%s%s", HOST_URL_CRYPTO,
                         crypto_to_fetch, currency);

                debug(INFO, "Fetching %s via proxy: %s:%s%s", crypto_to_fetch, PROXY_HOST,
                      PROXY_PORT, path);

                uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
                ReturnCode rc =
                    http_get(PROXY_HOST, proxy_port, path, crypto_response_callback, mtrx, is_first_fetch);
                if (RC_FAILED(rc))
                {
                    debug(ERROR, "HTTP request failed for %s: %s", crypto_to_fetch,
                          rc_to_string(rc));
                }

                g_crypto_to_fetch_index++;
            }

            g_crypto_to_fetch_index = 0;
            debug(INFO, "Crypto fetch complete");
        }
        else
        {
            debug(ERROR, "WiFi connection failed");
        }
    }

    if (g_data_fetched)
    {
        int total_width = 0;
        int crypto_widths[CRYPTO_COUNT];
        memset(crypto_widths, 0, sizeof(crypto_widths));

        for (unsigned int i = 0; i < CRYPTO_COUNT; i++)
        {
            if (!crypto_is_enabled(i))
            {
                continue;
            }

            const Crypto* crp = &CRYPTO_ARR[i];
            float current = crp->current_price;
            float open = crp->open_price;

            float change_perc = (open > 0.0f) ? ((current - open) / open) * 100 : 0.0f;

            char price_str[16];
            snprintf(price_str, sizeof(price_str), "%.2f", current);

            char change_perc_str[16];
            snprintf(change_perc_str, sizeof(change_perc_str), "%+.1f%%", change_perc);

            int width = matrix_calculate_word_width_with_space(crp->to_str) + 3 +
                        matrix_calculate_word_width_with_space(price_str) +
                        ICONS_ARR[TKR_UP].width +
                        matrix_calculate_word_width_with_space(change_perc_str) + 12;

            crypto_widths[i] = width;
            total_width += width;
        }

        matrix_clear(mtrx);

        int current_x = 0;
        for (unsigned int i = 0; i < CRYPTO_COUNT; i++)
        {
            if (!crypto_is_enabled(i))
            {
                continue;
            }

            const Crypto* crp = &CRYPTO_ARR[i];

            float current = crp->current_price;
            float open = crp->open_price;

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
                int crp_x = base_x + (wrap * total_width) + 2;
                int price_x = crp_x + matrix_calculate_word_width_with_space(crp->to_str) + 3;
                int icon_x = price_x + matrix_calculate_word_width_with_space(price_str);
                int perc_diff_x = icon_x + icon->width + 4;

                const unsigned int y = 1;

                matrix_display_word(crp->to_str, crp_x, y, &LABEL_ORANGE);
                matrix_display_word(price_str, price_x, y, colour);
                matrix_display_icon(icon, icon_x, y, colour);
                matrix_display_word(change_perc_str, perc_diff_x, y, colour);
            }

            current_x += crypto_widths[i];
        }

        matrix_show(mtrx);

        (*offset) += 2;
        if (*offset >= total_width)
        {
            *offset = 0;
        }
    }
}

void crypto_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_crypto_to_fetch_index = 0;
    g_scroll_offset = 0;
    g_url_scroll_offset = 0;
}
