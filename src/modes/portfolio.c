/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <portfolio.h>

#include <const.h>
#include <currency.h>
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

typedef struct
{
    char ticker[10];
    float quantity;
    float avg_price;
    float current_price;
    float prev_close;
    float csv_price;
} PortfolioPosition;

typedef enum
{
    METRIC_PL_ABSOLUTE = 0,
    METRIC_PL_PERCENTAGE,
    METRIC_BIGGEST_WINNER,
    METRIC_BIGGEST_LOSER,
    METRIC_TOTAL_VALUE,
    METRIC_COUNT
} PortfolioMetric;

static PortfolioPosition g_positions[50];
static uint8_t g_portfolio_count = 0;
static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_ticker_to_fetch_index = 0;
static int g_scroll_offset = 0;
static int g_url_scroll_offset = 0;
static PortfolioMetric g_current_metric = METRIC_PL_ABSOLUTE;

static void portfolio_response_callback(const char* body, size_t len, bool complete)
{
    debug(TRACE, "portfolio_response_callback: len=%d, complete=%d", len, complete);

    if (!complete) return;

    if (!body || len == 0)
    {
        debug(ERROR, "Empty response for portfolio ticker index %u", g_ticker_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    debug(DEBUG, "Received portfolio response: %.*s", (int)len, body);

    // Check if response contains an error
    if (strstr(body, "\"error\""))
    {
        debug(ERROR, "Proxy returned error for portfolio ticker index %u: %.*s",
              g_ticker_to_fetch_index, (int)len, body);
        g_positions[g_ticker_to_fetch_index].current_price = -1.0f;
        g_positions[g_ticker_to_fetch_index].prev_close = -1.0f;
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    float current_price = parse_price_from_json_array(body, "\"c\":");
    float prev_close = parse_price_from_json_string(body, "\"o\":");

    if (!current_price) current_price = 0.0f;
    if (!prev_close) prev_close = 0.0f;

    if (current_price > 0.0f)
    {
        g_positions[g_ticker_to_fetch_index].current_price = current_price;
        g_positions[g_ticker_to_fetch_index].prev_close = prev_close;
        g_data_fetched = true;

        // Throttle fetch interval based on portfolio size
        uint64_t fetch_interval_minutes;
        if (g_portfolio_count >= 30)
        {
            fetch_interval_minutes = 60;  // 1 hour for large portfolios (30+ tickers)
        }
        else if (g_portfolio_count >= 10)
        {
            fetch_interval_minutes = 30;  // 30 minutes for medium portfolios (10-29 tickers)
        }
        else
        {
            fetch_interval_minutes = 15;  // 15 minutes for small portfolios (<10 tickers)
        }

        uint64_t ms_until_next_fetch = fetch_interval_minutes * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed %s price: current=%.2f, prev_close=%.2f",
              g_positions[g_ticker_to_fetch_index].ticker, current_price, prev_close);
        debug(TRACE, "Next fetch scheduled in %llu minutes", fetch_interval_minutes);
    }
    else
    {
        debug(ERROR, "Failed to parse portfolio data for index %u", g_ticker_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
}

static void portfolio_fetch_prices(Matrix* mtrx)
{
    if (!wifi_is_connected() || g_portfolio_count == 0) return;

    device_config_t config;
    flash_memory_read(&config);
    const char* currency = currency_get_name(config.currency);

    debug(INFO, "Starting portfolio data fetch (currency: %s)", currency);

    while (g_ticker_to_fetch_index < g_portfolio_count)
    {
        const char* ticker = g_positions[g_ticker_to_fetch_index].ticker;

        char path[256];
        snprintf(path, sizeof(path), "/stocks/%s?currency=%s", ticker, currency);

        debug(INFO, "Fetching %s via proxy: %s:%s%s", ticker, PROXY_HOST, PROXY_PORT, path);

        uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
        ReturnCode rc = http_get(PROXY_HOST, proxy_port, path, portfolio_response_callback, mtrx);
        if (RC_FAILED(rc))
        {
            debug(ERROR, "HTTP request failed for %s: %s", ticker, rc_to_string(rc));
        }

        g_ticker_to_fetch_index++;
    }

    g_ticker_to_fetch_index = 0;
    g_data_fetched = true;
    debug(INFO, "Portfolio fetch complete");
}

static void portfolio_load_from_flash(void)
{
    device_config_t config;
    flash_memory_read(&config);

    if (config.portfolio_count > 0 && g_portfolio_count != config.portfolio_count)
    {
        g_portfolio_count = config.portfolio_count;
        for (uint8_t i = 0; i < g_portfolio_count; i++)
        {
            strncpy(g_positions[i].ticker, config.portfolio_tickers[i], 10);
            g_positions[i].quantity = config.portfolio_quantities[i];
            g_positions[i].avg_price = config.portfolio_avg_prices[i];
            g_positions[i].csv_price = config.portfolio_csv_prices[i];
            g_positions[i].current_price = g_positions[i].csv_price;
            g_positions[i].prev_close = g_positions[i].csv_price;
        }
        debug(INFO, "Loaded %d portfolio positions", g_portfolio_count);
    }
}

static void portfolio_calculate_metrics(float* total_pl, float* total_pct, int* winner_idx,
                                        int* loser_idx, float* total_value)
{
    device_config_t config;
    flash_memory_read(&config);
    bool daily_mode = (config.portfolio_mode == 1);

    float total_cost = 0.0f;
    *total_value = 0.0f;
    *winner_idx = -1;
    *loser_idx = -1;
    float max_pl = -999999.0f;
    float min_pl = 999999.0f;

    for (uint8_t i = 0; i < g_portfolio_count; i++)
    {
        // Skip positions with no valid data (error response or not yet fetched)
        if (g_positions[i].current_price <= 0.0f) continue;

        float baseline_price = daily_mode ? g_positions[i].prev_close : g_positions[i].avg_price;

        float cost = g_positions[i].quantity * baseline_price;
        float value = g_positions[i].quantity * g_positions[i].current_price;
        float pl = value - cost;

        total_cost += cost;
        *total_value += value;

        if (pl > max_pl)
        {
            max_pl = pl;
            *winner_idx = i;
        }
        if (pl < min_pl)
        {
            min_pl = pl;
            *loser_idx = i;
        }
    }

    *total_pl = *total_value - total_cost;
    *total_pct = (total_cost > 0.0f) ? (*total_pl / total_cost) * 100.0f : 0.0f;
}

static void portfolio_display_metric(Matrix* mtrx, PortfolioMetric metric)
{
    float total_pl, total_pct, total_value;
    int winner_idx, loser_idx;
    portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx, &total_value);

    device_config_t config;
    flash_memory_read(&config);
    const char* currency_symbol = currency_get_symbol(config.currency);

    char label_str[32] = "";
    char value_str[32] = "";
    const RGB* value_color = &DEFAULT_COLOUR;
    bool has_label = false;

    // Check if all tickers are invalid (no valid positions)
    bool all_invalid = (winner_idx == -1 && loser_idx == -1);

    switch (metric)
    {
        case METRIC_PL_ABSOLUTE:
            if (all_invalid)
            {
                snprintf(value_str, sizeof(value_str), "NO DATA");
                has_label = false;
            }
            else
            {
                snprintf(label_str, sizeof(label_str), "  P/L");
                snprintf(value_str, sizeof(value_str), " %.2f", total_pl);
                if (total_pl == 0.0f)
                    value_color = &DEFAULT_COLOUR;
                else
                    value_color = (total_pl > 0) ? &GREEN : &RED;
                has_label = true;
            }
            break;

        case METRIC_PL_PERCENTAGE:
            if (all_invalid)
            {
                snprintf(value_str, sizeof(value_str), "NO DATA");
                has_label = false;
            }
            else
            {
                snprintf(label_str, sizeof(label_str), "  P/L");
                snprintf(value_str, sizeof(value_str), " %.2f%%", total_pct);
                if (total_pct == 0.0f)
                    value_color = &DEFAULT_COLOUR;
                else
                    value_color = (total_pct > 0) ? &GREEN : &RED;
                has_label = true;
            }
            break;

        case METRIC_BIGGEST_WINNER:
            if (winner_idx >= 0)
            {
                device_config_t config;
                flash_memory_read(&config);
                bool daily_mode = (config.portfolio_mode == 1);
                float baseline = daily_mode ? g_positions[winner_idx].prev_close
                                            : g_positions[winner_idx].avg_price;

                float cost = g_positions[winner_idx].quantity * baseline;
                float current_value =
                    g_positions[winner_idx].quantity * g_positions[winner_idx].current_price;
                float pct_change = (cost > 0.0f) ? ((current_value - cost) / cost) * 100.0f : 0.0f;
                snprintf(label_str, sizeof(label_str), "  %s", g_positions[winner_idx].ticker);
                snprintf(value_str, sizeof(value_str), "%+.2f%%", pct_change);
                if (pct_change == 0.0f)
                    value_color = &DEFAULT_COLOUR;
                else
                    value_color = (pct_change > 0) ? &GREEN : &RED;
                has_label = true;
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "NO DATA");
                has_label = false;
            }
            break;

        case METRIC_BIGGEST_LOSER:
            if (loser_idx >= 0)
            {
                device_config_t config;
                flash_memory_read(&config);
                bool daily_mode = (config.portfolio_mode == 1);
                float baseline = daily_mode ? g_positions[loser_idx].prev_close
                                            : g_positions[loser_idx].avg_price;

                float cost = g_positions[loser_idx].quantity * baseline;
                float current_value =
                    g_positions[loser_idx].quantity * g_positions[loser_idx].current_price;
                float pct_change = (cost > 0.0f) ? ((current_value - cost) / cost) * 100.0f : 0.0f;
                snprintf(label_str, sizeof(label_str), "  %s", g_positions[loser_idx].ticker);
                snprintf(value_str, sizeof(value_str), "%+.2f%%", pct_change);
                if (pct_change == 0.0f)
                    value_color = &DEFAULT_COLOUR;
                else
                    value_color = (pct_change > 0) ? &GREEN : &RED;
                has_label = true;
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "NO DATA");
                has_label = false;
            }
            break;

        case METRIC_TOTAL_VALUE:
            if (all_invalid)
            {
                snprintf(value_str, sizeof(value_str), "NO DATA");
                has_label = false;
            }
            else
            {
                snprintf(label_str, sizeof(label_str), "  TOTAL");
                snprintf(value_str, sizeof(value_str), "%s%.2f %+.2f%%", currency_symbol,
                         total_value, total_pct);
                if (total_pct == 0.0f)
                    value_color = &DEFAULT_COLOUR;
                else
                    value_color = (total_pct > 0) ? &GREEN : &RED;
                has_label = true;
            }
            break;

        default:
            return;
    }

    int label_width = has_label ? matrix_calculate_word_width_with_space(label_str) : 0;
    int value_width = matrix_calculate_word_width_with_space(value_str);
    int total_width = label_width + value_width;
    int position_offset = 0;

    if (has_label)
    {
        if (metric == METRIC_PL_ABSOLUTE || metric == METRIC_PL_PERCENTAGE)
        {
            total_width -= 2;
            position_offset = -1;
        }
        else if (metric == METRIC_BIGGEST_WINNER || metric == METRIC_BIGGEST_LOSER)
        {
            total_width -= 1;
            position_offset = 2;
        }
        else if (metric == METRIC_TOTAL_VALUE)
        {
            total_width -= 1;
            position_offset = 1;
        }
    }

    matrix_clear(mtrx);

    if (total_width >= 31)
    {
        for (int wrap = 0; wrap < 2; wrap++)
        {
            int base_x = (wrap * (total_width + 4)) - g_scroll_offset;
            if (has_label)
            {
                matrix_display_word(label_str, base_x, 1, &LABEL_ORANGE);
                matrix_display_word(value_str, base_x + label_width + position_offset, 1,
                                    value_color);
            }
            else
            {
                matrix_display_word(value_str, base_x, 1, value_color);
            }
        }
        matrix_show(mtrx);

        g_scroll_offset += 2;
        if (g_scroll_offset >= total_width + 4)
        {
            g_scroll_offset = 0;
        }
    }
    else
    {
        int x_pos = 1;
        if (has_label)
        {
            matrix_display_word(label_str, x_pos, 1, &LABEL_ORANGE);
            matrix_display_word(value_str, x_pos + label_width + position_offset, 1, value_color);
        }
        else
        {
            matrix_display_word(value_str, x_pos, 1, value_color);
        }
        matrix_show(mtrx);
    }
}

static void portfolio_display_url(Matrix* mtrx)
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

void portfolio_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

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

    uint64_t current_time_ms = get_current_time_ms();
    bool should_fetch = (g_next_fetch_time_ms == 0) || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        portfolio_load_from_flash();
        if (g_portfolio_count > 0)
        {
            portfolio_fetch_prices(mtrx);
        }
    }

    if (g_portfolio_count == 0)
    {
        if (http_server_portfolio_parse_failed())
        {
            matrix_clear(mtrx);
            matrix_display_word("ERROR", 1, 1, &RED);
            matrix_show(mtrx);
        }
        else
        {
            portfolio_display_url(mtrx);
            g_url_scroll_offset++;
            int url_width =
                matrix_calculate_word_width_with_space("    http://photon.local/portfolio") + 4;
            if (g_url_scroll_offset >= url_width)
            {
                g_url_scroll_offset = 0;
            }
        }
    }
    else if (g_data_fetched)
    {
        portfolio_display_metric(mtrx, g_current_metric);
    }
    else
    {
        matrix_clear(mtrx);
        matrix_display_word("LOADING", 1, 1, &DEFAULT_COLOUR);
        matrix_show(mtrx);
    }
}

void portfolio_next_metric(Matrix* mtrx)
{
    ASSERT_NOT_NULL(mtrx);

    // Save current state before changing metric
    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    // Update to next metric
    g_current_metric = (g_current_metric + 1) % METRIC_COUNT;
    g_scroll_offset = 0;

    // Render new metric and save its state
    portfolio_display_metric(mtrx, g_current_metric);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    // Animate the transition
    matrix_slide_right(mtrx, current_state, next_state);
}

void portfolio_prev_metric(Matrix* mtrx)
{
    ASSERT_NOT_NULL(mtrx);

    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    if (g_current_metric == 0)
    {
        g_current_metric = METRIC_COUNT - 1;
    }
    else
    {
        g_current_metric--;
    }
    g_scroll_offset = 0;

    portfolio_display_metric(mtrx, g_current_metric);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_left(mtrx, current_state, next_state);
}

void portfolio_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_ticker_to_fetch_index = 0;
    g_scroll_offset = 0;
    g_url_scroll_offset = 0;
    g_current_metric = METRIC_PL_ABSOLUTE;
    debug(TRACE, "Portfolio cleanup called");
}
