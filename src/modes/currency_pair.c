/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <currency_pair.h>

#include <const.h>
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

CurrencyPairData g_currency_pairs[CURRENCY_PAIR_MAX] = {0};

static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_pair_to_fetch_index = 0;
static int g_scroll_offset = 0;
static int g_url_scroll_offset = 0;

// Parsing

static float parse_price_from_json(const char* json, const char* key)
{
    const char* pos = strstr(json, key);
    if (!pos) return 0.0f;

    pos += strlen(key);
    while (*pos && (*pos == ' ' || *pos == '"' || *pos == ':' || *pos == '[')) pos++;

    return atof(pos);
}

// Response callback

static void currency_pair_response_callback(const char* body, size_t len, bool complete)
{
    if (!complete) return;

    if (!body || len == 0)
    {
        debug(ERROR, "Empty response for currency pairs batch");
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    debug(DEBUG, "Received currency pairs batch response: %.*s", (int)len, body);

    // Parse batch response - iterate through all enabled pairs
    bool any_success = false;
    for (unsigned int i = 0; i < CURRENCY_PAIR_MAX; i++)
    {
        if (!g_currency_pairs[i].enabled || strlen(g_currency_pairs[i].pair) == 0) continue;

        // Build the search key for this pair (e.g., "GBPUSD=X")
        char search_key[16];
        snprintf(search_key, sizeof(search_key), "\"%s=X\"", g_currency_pairs[i].pair);

        // Find this pair's data in the batch response
        const char* pair_data = strstr(body, search_key);
        if (pair_data)
        {
            float current_price = parse_price_from_json(pair_data, "\"c\":");
            float open_price = parse_price_from_json(pair_data, "\"o\":");

            if (current_price > 0.0f && open_price > 0.0f)
            {
                g_currency_pairs[i].current_price = current_price;
                g_currency_pairs[i].open_price = open_price;
                any_success = true;

                debug(INFO, "Parsed %s: current: %.4f, open: %.4f", g_currency_pairs[i].pair,
                      current_price, open_price);
            }
            else
            {
                debug(WARN, "Failed to parse prices for %s", g_currency_pairs[i].pair);
            }
        }
        else
        {
            debug(WARN, "Pair %s not found in batch response", g_currency_pairs[i].pair);
        }
    }

    if (any_success)
    {
        g_data_fetched = true;
        uint64_t ms_until_next_fetch = TKR_FETCH_INTERVAL_MINUTES * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;
        debug(TRACE, "Next fetch scheduled in %d minutes", TKR_FETCH_INTERVAL_MINUTES);
    }
    else
    {
        debug(ERROR, "Failed to parse any currency pair data from batch");
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
}

static void currency_pair_display_url(Matrix* mtrx)
{
    const char* url_text = "    http://photon.local/watchlist";
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

void currency_pair_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    // Check if any currency pairs are configured
    uint8_t enabled_count = currency_pair_get_count();
    if (enabled_count == 0)
    {
        currency_pair_display_url(mtrx);
        g_url_scroll_offset++;
        int url_width =
            matrix_calculate_word_width_with_space("    http://photon.local/watchlist") + 4;
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
        debug(INFO, "Starting currency pair data fetch");

        if (wifi_is_connected())
        {
            device_config_t config;
            flash_memory_read(&config);

            // Load currency pairs from flash
            if (config.currency_pair_count > 0)
            {
                currency_pair_set_pairs(
                    (const char(*)[CURRENCY_PAIR_LEN + 1]) config.currency_pairs,
                    config.currency_pair_count);
            }

            // Build comma-separated list of enabled pairs
            char symbols[256] = "";
            uint8_t pair_count = 0;

            for (unsigned int i = 0; i < CURRENCY_PAIR_MAX; i++)
            {
                if (!g_currency_pairs[i].enabled || strlen(g_currency_pairs[i].pair) == 0)
                {
                    continue;
                }

                if (pair_count > 0)
                {
                    strcat(symbols, ",");
                }
                // Append "=X" for Yahoo Finance forex format
                strcat(symbols, g_currency_pairs[i].pair);
                strcat(symbols, "=X");
                pair_count++;
            }

            if (pair_count == 0)
            {
                debug(WARN, "No enabled currency pairs to fetch");
                g_next_fetch_time_ms = current_time_ms + (60 * 1000);  // Retry in 1 minute
                return;
            }

            // Use batch endpoint
            char path[512];
            snprintf(path, sizeof(path), "/stocks/batch?symbols=%s", symbols);

            debug(INFO, "Fetching %u currency pairs via batch: %s:%s%s", pair_count, PROXY_HOST,
                  PROXY_PORT, path);

            uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);

            // Parse the batch response
            g_pair_to_fetch_index = 0;
            ReturnCode rc = http_get(PROXY_HOST, proxy_port, path, currency_pair_response_callback,
                                     mtrx, is_first_fetch);

            if (RC_FAILED(rc))
            {
                debug(ERROR, "HTTP request failed for currency pairs: %s", rc_to_string(rc));
                g_next_fetch_time_ms = current_time_ms + (60 * 1000);  // Retry in 1 minute
            }
        }
        else
        {
            debug(ERROR, "WiFi connection failed");
            g_next_fetch_time_ms = current_time_ms + (60 * 1000);  // Retry in 1 minute
        }
    }

    if (!g_data_fetched) return;

    // Display currency pairs
    int total_width = 0;
    int pair_widths[CURRENCY_PAIR_MAX];
    memset(pair_widths, 0, sizeof(pair_widths));

    // Calculate total width
    for (unsigned int i = 0; i < CURRENCY_PAIR_MAX; i++)
    {
        if (!g_currency_pairs[i].enabled || strlen(g_currency_pairs[i].pair) == 0) continue;

        float current = g_currency_pairs[i].current_price;
        float open = g_currency_pairs[i].open_price;

        if (current == 0.0f || open == 0.0f) continue;

        // Format pair name (e.g., "GBP/USD")
        char pair_label[8];
        snprintf(pair_label, sizeof(pair_label), "%.3s/%.3s", g_currency_pairs[i].pair,
                 g_currency_pairs[i].pair + 3);

        // Format rate (e.g., "1.3704")
        char rate_str[16];
        snprintf(rate_str, sizeof(rate_str), "%.4f", current);

        // Format percentage (e.g., "+0.81%")
        float pct_change = ((current - open) / open) * 100.0f;
        char change_perc_str[16];
        snprintf(change_perc_str, sizeof(change_perc_str), "%+.2f%%", pct_change);

        int width = matrix_calculate_word_width_with_space(pair_label) + 3 +
                    matrix_calculate_word_width_with_space(rate_str) + ICONS_ARR[TKR_UP].width +
                    matrix_calculate_word_width_with_space(change_perc_str) + 12;

        pair_widths[i] = width;
        total_width += width;
    }

    if (total_width == 0)
    {
        matrix_clear(mtrx);
        matrix_display_word("NO DATA", 1, 1, &DEFAULT_COLOUR);
        matrix_show(mtrx);
        return;
    }

    matrix_clear(mtrx);

    int current_x = 0;
    for (unsigned int i = 0; i < CURRENCY_PAIR_MAX; i++)
    {
        if (!g_currency_pairs[i].enabled || strlen(g_currency_pairs[i].pair) == 0) continue;

        float current = g_currency_pairs[i].current_price;
        float open = g_currency_pairs[i].open_price;

        if (current == 0.0f || open == 0.0f) continue;

        float pct_change = ((current - open) / open) * 100.0f;
        bool is_up = pct_change >= 0.0f;

        const Glyph* icon = is_up ? &ICONS_ARR[TKR_UP] : &ICONS_ARR[TKR_DOWN];
        const RGB* colour = is_up ? &GREEN : &RED;

        // Format pair name (e.g., "GBP/USD")
        char pair_label[8];
        snprintf(pair_label, sizeof(pair_label), "%.3s/%.3s", g_currency_pairs[i].pair,
                 g_currency_pairs[i].pair + 3);

        // Format rate (e.g., "1.3704")
        char rate_str[16];
        snprintf(rate_str, sizeof(rate_str), "%.4f", current);

        // Format percentage (e.g., "+0.81%")
        char change_perc_str[16];
        snprintf(change_perc_str, sizeof(change_perc_str), "%+.2f%%", pct_change);

        int base_x = current_x - *offset;

        for (int wrap = 0; wrap < 2; wrap++)
        {
            int pair_x = base_x + (wrap * total_width) + 2;
            int rate_x = pair_x + matrix_calculate_word_width_with_space(pair_label) + 3;
            int icon_x = rate_x + matrix_calculate_word_width_with_space(rate_str);
            int perc_x = icon_x + icon->width + 4;

            const unsigned int y = 1;

            matrix_display_word(pair_label, pair_x, y, &LABEL_ORANGE);
            matrix_display_word(rate_str, rate_x, y, colour);
            matrix_display_icon(icon, icon_x, y, colour);
            matrix_display_word(change_perc_str, perc_x, y, colour);
        }

        current_x += pair_widths[i];
    }

    matrix_show(mtrx);

    (*offset) += 2;
    if (*offset >= total_width)
    {
        *offset = 0;
    }
}

void currency_pair_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_scroll_offset = 0;
    g_url_scroll_offset = 0;
    g_pair_to_fetch_index = 0;
}

uint8_t currency_pair_get_count(void)
{
    uint8_t count = 0;
    for (unsigned int i = 0; i < CURRENCY_PAIR_MAX; i++)
    {
        if (g_currency_pairs[i].enabled && strlen(g_currency_pairs[i].pair) > 0)
        {
            count++;
        }
    }
    return count;
}

void currency_pair_set_pairs(const char pairs[][CURRENCY_PAIR_LEN + 1], uint8_t count)
{
    // Clear existing
    memset(g_currency_pairs, 0, sizeof(g_currency_pairs));

    // Set new pairs
    for (uint8_t i = 0; i < count && i < CURRENCY_PAIR_MAX; i++)
    {
        if (strlen(pairs[i]) == CURRENCY_PAIR_LEN)
        {
            memcpy(g_currency_pairs[i].pair, pairs[i], CURRENCY_PAIR_LEN);
            g_currency_pairs[i].pair[CURRENCY_PAIR_LEN] = '\0';
            g_currency_pairs[i].enabled = true;
            g_currency_pairs[i].current_price = 0.0f;
            g_currency_pairs[i].open_price = 0.0f;
        }
    }

    // Reset fetch state when pairs change
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
}

void currency_pair_get_pairs(char pairs[][CURRENCY_PAIR_LEN + 1], uint8_t* count)
{
    *count = 0;
    for (unsigned int i = 0; i < CURRENCY_PAIR_MAX; i++)
    {
        if (g_currency_pairs[i].enabled && strlen(g_currency_pairs[i].pair) > 0)
        {
            memcpy(pairs[*count], g_currency_pairs[i].pair, CURRENCY_PAIR_LEN);
            pairs[*count][CURRENCY_PAIR_LEN] = '\0';
            (*count)++;
        }
    }
}
