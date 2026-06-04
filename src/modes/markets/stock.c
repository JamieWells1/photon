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
static volatile uint64_t g_fetch_start_ms = 0;
static int g_scroll_offset = 0;
static int g_url_scroll_offset = 0;

void stock_invalidate_cache(void)
{
    g_next_fetch_time_ms = 0;
    g_data_fetched = false;
    g_tickers_loaded = false;
}

uint8_t stock_get_count(void)
{
    if (!g_tickers_loaded)
    {
        device_config_t config;
        flash_memory_read(&config);

        g_stock_count = 0;

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

            if (config.portfolio_count == 0)
            {
                g_enabled_mask = config.stocks ? config.stocks : ((1ULL << g_stock_count) - 1);
            }
            else
            {
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

static void stock_response_callback(const char* body, size_t len, bool complete)
{
    print(TRACE, "stock_response_callback: len=%zu, complete=%d", len, complete);

    if (!complete) return;

    uint64_t elapsed_ms = get_current_time_ms() - g_fetch_start_ms;
    print(INFO, "Batch response received: %zu bytes in %llums", len, (unsigned long long)elapsed_ms);

    if (!body || len == 0)
    {
        print(ERROR, "Empty batch response after %llums", (unsigned long long)elapsed_ms);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    print(DEBUG, "Received batch stock response: %.*s", (int)len, body);

    // Parse batch response - iterate through all stocks and extract their data
    uint8_t parsed_count = 0;
    for (uint8_t i = 0; i < g_stock_count; i++)
    {
        if (!stock_is_enabled(i)) continue;

        const char* ticker = g_stocks[i].ticker;

        // Search for ticker key in JSON: "AAPL":
        char ticker_search[32];
        snprintf(ticker_search, sizeof(ticker_search), "\"%.10s\":", ticker);

        const char* ticker_data = strstr(body, ticker_search);
        if (!ticker_data)
        {
            print(WARN, "Ticker %s not found in batch response", ticker);
            g_stocks[i].current_price = -1.0f;
            g_stocks[i].open_price = -1.0f;
            continue;
        }

        // Parse current price and prev_close (using Kraken format: "c" and "o")
        float current_price = parse_price_from_json_array(ticker_data, "\"c\":");
        float open_price = parse_price_from_json_string(ticker_data, "\"o\":");

        if (!current_price) current_price = 0.0f;
        if (!open_price) open_price = 0.0f;

        if (current_price > 0.0f && open_price > 0.0f)
        {
            g_stocks[i].current_price = current_price;
            g_stocks[i].open_price = open_price;
            parsed_count++;
            print(TRACE, "%s: current=$%.2f, open=$%.2f", ticker, current_price, open_price);
        }
        else
        {
            print(WARN, "Failed to parse data for %s", ticker);
            g_stocks[i].current_price = -1.0f;
            g_stocks[i].open_price = -1.0f;
        }
    }

    if (parsed_count > 0)
    {
        g_data_fetched = true;

        uint8_t enabled_count = 0;
        for (uint8_t i = 0; i < g_stock_count; i++)
        {
            if (stock_is_enabled(i)) enabled_count++;
        }

        uint64_t ms_until_next_fetch = TKR_FETCH_INTERVAL_MINUTES * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        print(INFO, "Batch fetch complete: %u/%u tickers parsed in %llums",
              parsed_count, enabled_count, (unsigned long long)elapsed_ms);
        print(INFO, "Next fetch in %d min (T+%llums)",
              TKR_FETCH_INTERVAL_MINUTES, (unsigned long long)g_next_fetch_time_ms);
    }
    else
    {
        print(ERROR, "Failed to parse any stock data from batch response (%llums elapsed)",
              (unsigned long long)elapsed_ms);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
}

static void stock_display_url(Matrix* mtrx)
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

void stock_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    stock_get_count();

    if (g_stock_count == 0)
    {
        stock_display_url(mtrx);
        g_url_scroll_offset += 2;
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
        if (is_first_fetch)
            print(INFO, "Batch fetch triggered: first fetch since boot");
        else
            print(INFO, "Batch fetch triggered: cache expired at T+%llums", (unsigned long long)current_time_ms);

        if (wifi_is_connected())
        {
            // Build comma-separated list of enabled tickers
            char symbols[512] = "";
            uint8_t ticker_count = 0;

            for (uint8_t i = 0; i < g_stock_count; i++)
            {
                if (!stock_is_enabled(i)) continue;

                if (ticker_count > 0)
                {
                    strcat(symbols, ",");
                }
                strcat(symbols, g_stocks[i].ticker);
                ticker_count++;
            }

            if (ticker_count == 0)
            {
                print(WARN, "No enabled stocks to fetch");
                return;
            }

            // Build batch request path
            char path[768];
            snprintf(path, sizeof(path), "/stocks/batch?symbols=%s", symbols);

            print(INFO, "Batch request: %u/%u tickers enabled, %zu byte URL",
                  ticker_count, g_stock_count, strlen(path));
            print(INFO, "Symbols: %s", symbols);
            print(INFO, "Dispatching GET %s:%s%s", PROXY_HOST, PROXY_PORT, path);

            g_fetch_start_ms = get_current_time_ms();

            uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
            ReturnCode rc = http_get(PROXY_HOST, proxy_port, path, stock_response_callback, mtrx,
                                     is_first_fetch);
            if (RC_FAILED(rc))
            {
                print(ERROR, "Batch HTTP request failed immediately: %s", rc_to_string(rc));
            }
            else
            {
                print(INFO, "Batch HTTP request accepted by TCP stack");
            }
        }
        else
        {
            print(ERROR, "WiFi not connected — batch fetch aborted");
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

            if (current < 0.0f || open < 0.0f)
            {
                width = matrix_calculate_word_width_with_space(stk->ticker) + 3 +
                        matrix_calculate_word_width_with_space("NO DATA") + 12;
            }
            else
            {
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

            if (current < 0.0f || open < 0.0f)
            {
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
    print(INFO, "Ticker validation result: %s", valid ? "valid" : "invalid");
}

void stock_validate_pending_ticker(Matrix* mtrx)
{
    const char* ticker = http_server_get_pending_ticker();
    if (!ticker || ticker[0] == '\0') return;

    char path[256];
    snprintf(path, sizeof(path), "/stocks/%s", ticker);

    print(INFO, "Validating ticker %s via proxy", ticker);

    uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
    ReturnCode rc =
        http_get(PROXY_HOST, proxy_port, path, validation_response_callback, mtrx, true);
    if (RC_FAILED(rc))
    {
        print(ERROR, "Ticker validation request failed for %s: %s", ticker, rc_to_string(rc));
        http_server_set_validation_result(false);
    }
}

void stock_cleanup(void)
{
    g_scroll_offset = 0;
    g_url_scroll_offset = 0;
    g_tickers_loaded = false;
}
