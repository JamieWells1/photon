/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <stock.h>

#include <const.h>
#include <error.h>
#include <flash_memory.h>
#include <glyphs.h>
#include <http.h>
#include <http_server.h>
#include <macro.h>
#include <matrix.h>
#include <utils.h>
#include <wifi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>

static Stock g_stocks[MAX_STOCK_TICKERS];
static uint8_t g_stock_count = 0;
static uint64_t g_enabled_mask = 0;
static bool g_tickers_loaded = false;

static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_stk_to_fetch_index = 0;
static int g_scroll_offset = 0;
static int g_url_scroll_offset = 0;

uint8_t stock_get_count(void)
{
    if (!g_tickers_loaded)
    {
        device_config_t config;
        flash_memory_read(&config);

        g_stock_count = 0;

        // Load portfolio tickers first
        if (config.portfolio_count > 0)
        {
            for (uint8_t i = 0; i < config.portfolio_count && i < MAX_STOCK_TICKERS; i++)
            {
                strncpy(g_stocks[g_stock_count].ticker, config.portfolio_tickers[i], 10);
                g_stocks[g_stock_count].current_price = 0.0f;
                g_stocks[g_stock_count].open_price = 0.0f;
                g_stock_count++;
            }
            g_enabled_mask = config.stocks ? config.stocks : ((1ULL << g_stock_count) - 1);
        }

        // Also load manual tickers (can coexist with portfolio)
        if (config.stock_ticker_count > 0 && g_stock_count < MAX_STOCK_TICKERS)
        {
            uint8_t manual_start_index = g_stock_count;
            for (uint8_t i = 0; i < config.stock_ticker_count && g_stock_count < MAX_STOCK_TICKERS;
                 i++)
            {
                strncpy(g_stocks[g_stock_count].ticker, config.stock_tickers[i], 10);
                g_stocks[g_stock_count].current_price = 0.0f;
                g_stocks[g_stock_count].open_price = 0.0f;
                g_stock_count++;
            }

            // Manual tickers are always enabled
            if (config.portfolio_count == 0)
            {
                // No portfolio: enabled mask covers manual tickers only
                g_enabled_mask = config.stocks ? config.stocks : ((1ULL << g_stock_count) - 1);
            }
            else
            {
                // Has portfolio: enable manual ticker bits (after portfolio ticker bits)
                for (uint8_t i = manual_start_index; i < g_stock_count; i++)
                {
                    g_enabled_mask |= (1ULL << i);
                }
            }
        }

        g_tickers_loaded = true;
    }

    return g_stock_count;
}

const char* stock_get_ticker(int idx)
{
    if (idx < 0 || idx >= g_stock_count) return "";
    return g_stocks[idx].ticker;
}

void stock_set_enabled_mask(uint64_t mask) { g_enabled_mask = mask; }

static bool stock_is_enabled(int idx)
{
    if (idx < 0 || idx >= g_stock_count) return false;
    return (g_enabled_mask & (1ULL << idx)) != 0;
}

// Display logic

static void stock_response_callback(const char* body, size_t len, bool complete)
{
    debug(TRACE, "stock_response_callback: len=%d, complete=%d", len, complete);

    if (!complete) return;

    if (!body || len == 0)
    {
        debug(ERROR, "Empty response for stock index %u", g_stk_to_fetch_index);

        // Check if this is the first failure (prices not yet set to -1.0f)
        bool first_failure = (g_stocks[g_stk_to_fetch_index].current_price != -1.0f);

        g_stocks[g_stk_to_fetch_index].current_price = -1.0f;
        g_stocks[g_stk_to_fetch_index].open_price = -1.0f;

        // Only set 60s retry on first failure, then accept it and use 15min cycle
        if (first_failure && g_next_fetch_time_ms == 0)
        {
            uint64_t ms_until_retry = 60 * 1000;
            g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
            debug(INFO, "First failure for ticker, will retry in 60s");
        }
        return;
    }

    debug(DEBUG, "Received stock response: %.*s", (int)len, body);

    // Check if response contains an error
    if (strstr(body, "\"error\""))
    {
        debug(ERROR, "Proxy returned error for stock index %u: %.*s", g_stk_to_fetch_index,
              (int)len, body);

        // Check if this is the first failure (prices not yet set to -1.0f)
        bool first_failure = (g_stocks[g_stk_to_fetch_index].current_price != -1.0f);

        g_stocks[g_stk_to_fetch_index].current_price = -1.0f;
        g_stocks[g_stk_to_fetch_index].open_price = -1.0f;

        // Only set 60s retry on first failure, then accept it and use 15min cycle
        if (first_failure && g_next_fetch_time_ms == 0)
        {
            uint64_t ms_until_retry = 60 * 1000;
            g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
            debug(INFO, "First failure for ticker, will retry in 60s");
        }
        return;
    }

    float current_price = parse_price_from_json_array(body, "\"c\":");
    float open_price = parse_price_from_json_string(body, "\"o\":");

    if (!current_price) current_price = 0.0f;
    if (!open_price) open_price = 0.0f;

    if (current_price > 0.0f && open_price > 0.0f)
    {
        g_stocks[g_stk_to_fetch_index].current_price = current_price;
        g_stocks[g_stk_to_fetch_index].open_price = open_price;
        g_data_fetched = true;

        // Count enabled tickers for throttling
        uint8_t enabled_count = 0;
        for (uint8_t i = 0; i < g_stock_count; i++)
        {
            if (stock_is_enabled(i)) enabled_count++;
        }

        // Throttle fetch interval based on number of enabled tickers
        uint64_t fetch_interval_minutes;
        if (enabled_count >= 30)
        {
            fetch_interval_minutes = 60;  // 1 hour for large ticker lists (30+ tickers)
        }
        else if (enabled_count >= 10)
        {
            fetch_interval_minutes = 30;  // 30 minutes for medium ticker lists (10-29 tickers)
        }
        else
        {
            fetch_interval_minutes = 15;  // 15 minutes for small ticker lists (<10 tickers)
        }

        uint64_t ms_until_next_fetch = fetch_interval_minutes * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed current: $%.2f, open: $%.2f", current_price, open_price);
        debug(TRACE, "Next fetch scheduled in %llu minutes (%u enabled tickers)",
              fetch_interval_minutes, enabled_count);
    }
    else
    {
        debug(ERROR, "Failed to parse stock data for index %u", g_stk_to_fetch_index);

        // Check if this is the first failure (prices not yet set to -1.0f)
        bool first_failure = (g_stocks[g_stk_to_fetch_index].current_price != -1.0f);

        g_stocks[g_stk_to_fetch_index].current_price = -1.0f;
        g_stocks[g_stk_to_fetch_index].open_price = -1.0f;

        // Only set 60s retry on first failure, then accept it and use 15min cycle
        if (first_failure && g_next_fetch_time_ms == 0)
        {
            uint64_t ms_until_retry = 60 * 1000;
            g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
            debug(INFO, "First failure for ticker, will retry in 60s");
        }
    }
}

static void stock_display_url(Matrix* mtrx)
{
    const char* url_text = "    http://photon.local/portfolio";
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

void stock_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    // Handle config save reboot
    if (http_server_config_saved())
    {
        debug(INFO, "Config saved, rebooting...");

        matrix_clear(mtrx);
        matrix_display_word_icon_pair("REBOOT...", &ORANGE, NULL, 0);
        matrix_show(mtrx);

        sleep_ms(2000);

        watchdog_enable(1, 1);
        while (1);
    }

    // Load tickers if not yet loaded
    stock_get_count();

    // No tickers available — show portfolio upload URL
    if (g_stock_count == 0)
    {
        stock_display_url(mtrx);
        g_url_scroll_offset++;
        int url_width =
            matrix_calculate_word_width_with_space("    http://photon.local/portfolio") + 4;
        if (g_url_scroll_offset >= url_width)
        {
            g_url_scroll_offset = 0;
        }

        return;
    }

    // Fetch stock data
    uint64_t current_time_ms = get_current_time_ms();
    bool should_fetch = (g_next_fetch_time_ms == 0) || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        debug(INFO, "Starting stock data fetch");

        if (wifi_is_connected())
        {
            while (g_stk_to_fetch_index < g_stock_count)
            {
                if (!stock_is_enabled(g_stk_to_fetch_index))
                {
                    g_stk_to_fetch_index++;
                    continue;
                }

                const char* stk_to_fetch = g_stocks[g_stk_to_fetch_index].ticker;

                char path[256];
                snprintf(path, sizeof(path), "/stocks/%s", stk_to_fetch);

                debug(INFO, "Fetching %s via proxy: %s:%s%s", stk_to_fetch, PROXY_HOST, PROXY_PORT,
                      path);

                uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
                ReturnCode rc =
                    http_get(PROXY_HOST, proxy_port, path, stock_response_callback, mtrx);
                if (RC_FAILED(rc))
                {
                    debug(ERROR, "HTTP request failed for %s: %s", stk_to_fetch, rc_to_string(rc));
                }

                g_stk_to_fetch_index++;
            }

            g_stk_to_fetch_index = 0;
            debug(INFO, "Stock fetch complete");
        }
        else
        {
            debug(ERROR, "WiFi connection failed");
        }
    }

    if (g_data_fetched)
    {
        int total_width = 0;
        int stock_widths[MAX_STOCK_TICKERS];
        memset(stock_widths, 0, sizeof(stock_widths));

        for (int i = 0; i < g_stock_count; i++)
        {
            if (!stock_is_enabled(i)) continue;

            const Stock* stk = &g_stocks[i];
            float current = stk->current_price;
            float open = stk->open_price;

            int width;

            // Check if ticker has invalid data (error response)
            if (current < 0.0f || open < 0.0f)
            {
                // Calculate width for "TICKER NO DATA" display
                width = matrix_calculate_word_width_with_space(stk->ticker) + 3 +
                        matrix_calculate_word_width_with_space("NO DATA") + 12;
            }
            else
            {
                // Calculate width for normal display with price and percentage
                float change_perc = (open > 0.0f) ? ((current - open) / open) * 100 : 0.0f;

                char price_str[16];
                snprintf(price_str, sizeof(price_str), "%.2f", current);

                char change_perc_str[16];
                snprintf(change_perc_str, sizeof(change_perc_str), "%+.1f%%", change_perc);

                width = matrix_calculate_word_width_with_space(stk->ticker) + 3 +
                            matrix_calculate_word_width_with_space(price_str) +
                            ICONS_ARR[TKR_UP].width +
                            matrix_calculate_word_width_with_space(change_perc_str) + 12;
            }

            stock_widths[i] = width;
            total_width += width;
        }

        matrix_clear(mtrx);

        int current_x = 0;
        for (int i = 0; i < g_stock_count; i++)
        {
            if (!stock_is_enabled(i)) continue;

            const Stock* stk = &g_stocks[i];

            float current = stk->current_price;
            float open = stk->open_price;

            int base_x = current_x - *offset;

            // Check if ticker has invalid data
            if (current < 0.0f || open < 0.0f)
            {
                // Display ticker name with "NO DATA" message
                for (int wrap = 0; wrap < 2; wrap++)
                {
                    int stk_x = base_x + (wrap * total_width) + 2;
                    int no_data_x = stk_x + matrix_calculate_word_width_with_space(stk->ticker) + 3;

                    const unsigned int y = 1;

                    matrix_display_word(stk->ticker, stk_x, y, &LABEL_ORANGE);
                    matrix_display_word("NO DATA", no_data_x, y, &RED);
                }
            }
            else
            {
                // Display normal ticker with price and percentage
                float change_perc = ((current - open) / open) * 100;
                bool is_up = (change_perc >= 0.0f);

                const Glyph* icon = is_up ? &ICONS_ARR[TKR_UP] : &ICONS_ARR[TKR_DOWN];
                const RGB* colour = is_up ? &GREEN : &RED;

                char price_str[16];
                snprintf(price_str, sizeof(price_str), "%.2f", current);

                char change_perc_str[16];
                snprintf(change_perc_str, sizeof(change_perc_str), "%+.1f%%", change_perc);

                for (int wrap = 0; wrap < 2; wrap++)
                {
                    int stk_x = base_x + (wrap * total_width) + 2;
                    int price_x = stk_x + matrix_calculate_word_width_with_space(stk->ticker) + 3;
                    int icon_x = price_x + matrix_calculate_word_width_with_space(price_str);
                    int perc_diff_x = icon_x + icon->width + 4;

                    const unsigned int y = 1;

                    matrix_display_word(stk->ticker, stk_x, y, &LABEL_ORANGE);
                    matrix_display_word(price_str, price_x, y, colour);
                    matrix_display_icon(icon, icon_x, y, colour);
                    matrix_display_word(change_perc_str, perc_diff_x, y, colour);
                }
            }

            current_x += stock_widths[i];
        }

        matrix_show(mtrx);

        (*offset) += 2;
        if (*offset >= total_width)
        {
            *offset = 0;
        }
    }
}

static void validation_response_callback(const char* body, size_t len, bool complete)
{
    if (!complete) return;

    bool valid = false;
    if (body && len > 0)
    {
        float price = parse_price_from_json_array(body, "\"c\":");
        valid = (price > 0.0f);
    }

    http_server_set_validation_result(valid);
    debug(INFO, "Ticker validation result: %s", valid ? "valid" : "invalid");
}

void stock_validate_pending_ticker(Matrix* mtrx)
{
    const char* ticker = http_server_get_pending_ticker();
    if (!ticker || ticker[0] == '\0') return;

    char path[256];
    snprintf(path, sizeof(path), "/stocks/%s", ticker);

    debug(INFO, "Validating ticker %s via proxy", ticker);

    uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
    ReturnCode rc = http_get(PROXY_HOST, proxy_port, path, validation_response_callback, mtrx);
    if (RC_FAILED(rc))
    {
        debug(ERROR, "Ticker validation request failed for %s: %s", ticker, rc_to_string(rc));
        http_server_set_validation_result(false);
    }
}

void stock_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_stk_to_fetch_index = 0;
    g_scroll_offset = 0;
    g_url_scroll_offset = 0;
    g_stock_count = 0;
    g_tickers_loaded = false;
    g_enabled_mask = 0;
}
