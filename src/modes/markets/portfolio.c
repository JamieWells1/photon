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
static int g_scroll_offset = 0;
static int g_url_scroll_offset = 0;
static PortfolioMetric g_current_metric = METRIC_PL_ABSOLUTE;
static int g_auto_scroll_offset = 0;
static bool g_scroll_type_loaded = false;
static uint8_t g_scroll_type = 0;  // 0 = automatic (default), 1 = manual

static void portfolio_response_callback(const char* body, size_t len, bool complete)
{
    print(TRACE, "portfolio_response_callback: len=%d, complete=%d", len, complete);

    if (!complete) return;

    if (!body || len == 0)
    {
        print(ERROR, "Empty batch response for portfolio");
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    print(DEBUG, "Received batch response: %.*s", (int)len, body);

    for (uint8_t i = 0; i < g_portfolio_count; i++)
    {
        const char* ticker = g_positions[i].ticker;
        char ticker_search[32];
        snprintf(ticker_search, sizeof(ticker_search), "\"%.10s\":", ticker);

        const char* ticker_data = strstr(body, ticker_search);
        if (!ticker_data)
        {
            print(WARN, "Ticker %s not found in batch response", ticker);
            g_positions[i].current_price = -1.0f;
            g_positions[i].prev_close = -1.0f;
            continue;
        }

        if (strstr(ticker_data, "\"error\""))
        {
            print(ERROR, "Error for ticker %s in batch response", ticker);
            g_positions[i].current_price = -1.0f;
            g_positions[i].prev_close = -1.0f;
            continue;
        }

        float current_price = parse_price_from_json_array(ticker_data, "\"c\":");
        float prev_close = parse_price_from_json_string(ticker_data, "\"o\":");

        if (!current_price) current_price = 0.0f;
        if (!prev_close) prev_close = 0.0f;

        if (current_price > 0.0f)
        {
            g_positions[i].current_price = current_price;
            g_positions[i].prev_close = prev_close;
            print(INFO, "Parsed %s: current=%.2f, prev_close=%.2f", ticker, current_price,
                  prev_close);
        }
        else
        {
            print(ERROR, "Failed to parse price for %s", ticker);
            g_positions[i].current_price = -1.0f;
            g_positions[i].prev_close = -1.0f;
        }
    }

    g_data_fetched = true;

    uint64_t ms_until_next_fetch = TKR_FETCH_INTERVAL_MINUTES * 60 * 1000;
    g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;
    print(INFO, "Batch fetch complete. Next fetch in %d minutes", TKR_FETCH_INTERVAL_MINUTES);
}

static void portfolio_fetch_prices(Matrix* mtrx, bool is_first_fetch)
{
    if (!wifi_is_connected() || g_portfolio_count == 0) return;

    device_config_t config;
    flash_memory_read(&config);
    const char* currency = currency_get_name(config.currency);

    char symbols[512] = "";
    for (uint8_t i = 0; i < g_portfolio_count; i++)
    {
        if (i > 0)
        {
            strcat(symbols, ",");
        }
        strcat(symbols, g_positions[i].ticker);
    }

    char path[768];
    snprintf(path, sizeof(path), "/stocks/batch?symbols=%s&currency=%s", symbols, currency);

    print(INFO, "Batch fetching %u tickers via proxy: %s:%s%s", g_portfolio_count, PROXY_HOST,
          PROXY_PORT, path);

    uint16_t proxy_port = (uint16_t)atoi(PROXY_PORT);
    ReturnCode rc =
        http_get(PROXY_HOST, proxy_port, path, portfolio_response_callback, mtrx, is_first_fetch);
    if (RC_FAILED(rc))
    {
        print(ERROR, "Batch HTTP request failed: %s", rc_to_string(rc));
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
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
        print(INFO, "Loaded %d portfolio positions", g_portfolio_count);
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

typedef struct
{
    char label[32];
    char value[32];
    const RGB* color;
    bool has_label;
    int position_offset;
} MetricRenderData;

static void portfolio_build_metric_data(PortfolioMetric metric, MetricRenderData* out,
                                        float total_pl, float total_pct, float total_value,
                                        int winner_idx, int loser_idx, const char* currency_symbol,
                                        bool daily_mode)
{
    out->label[0] = '\0';
    out->value[0] = '\0';
    out->color = &DEFAULT_COLOUR;
    out->has_label = false;
    out->position_offset = 0;

    bool all_invalid = (winner_idx == -1 && loser_idx == -1);

    switch (metric)
    {
        case METRIC_PL_ABSOLUTE:
            if (all_invalid)
            {
                snprintf(out->value, sizeof(out->value), "NO DATA");
            }
            else
            {
                snprintf(out->label, sizeof(out->label), "P/L");
                snprintf(out->value, sizeof(out->value), " %.2f", total_pl);
                if (total_pl != 0.0f) out->color = (total_pl > 0) ? &GREEN : &RED;
                out->has_label = true;
                out->position_offset = -1;
            }
            break;

        case METRIC_PL_PERCENTAGE:
            if (all_invalid)
            {
                snprintf(out->value, sizeof(out->value), "NO DATA");
            }
            else
            {
                snprintf(out->label, sizeof(out->label), "P/L");
                snprintf(out->value, sizeof(out->value), " %.2f%%", total_pct);
                if (total_pct != 0.0f) out->color = (total_pct > 0) ? &GREEN : &RED;
                out->has_label = true;
                out->position_offset = -1;
            }
            break;

        case METRIC_BIGGEST_WINNER:
            if (winner_idx >= 0)
            {
                float baseline = daily_mode ? g_positions[winner_idx].prev_close
                                            : g_positions[winner_idx].avg_price;
                float cost = g_positions[winner_idx].quantity * baseline;
                float cur_val =
                    g_positions[winner_idx].quantity * g_positions[winner_idx].current_price;
                float pct = (cost > 0.0f) ? ((cur_val - cost) / cost) * 100.0f : 0.0f;
                snprintf(out->label, sizeof(out->label), "  %s", g_positions[winner_idx].ticker);
                snprintf(out->value, sizeof(out->value), "%+.2f%%", pct);
                if (pct != 0.0f) out->color = (pct > 0) ? &GREEN : &RED;
                out->has_label = true;
                out->position_offset = 2;
            }
            else
            {
                snprintf(out->value, sizeof(out->value), "NO DATA");
            }
            break;

        case METRIC_BIGGEST_LOSER:
            if (loser_idx >= 0)
            {
                float baseline = daily_mode ? g_positions[loser_idx].prev_close
                                            : g_positions[loser_idx].avg_price;
                float cost = g_positions[loser_idx].quantity * baseline;
                float cur_val =
                    g_positions[loser_idx].quantity * g_positions[loser_idx].current_price;
                float pct = (cost > 0.0f) ? ((cur_val - cost) / cost) * 100.0f : 0.0f;
                snprintf(out->label, sizeof(out->label), "  %s", g_positions[loser_idx].ticker);
                snprintf(out->value, sizeof(out->value), "%+.2f%%", pct);
                if (pct != 0.0f) out->color = (pct > 0) ? &GREEN : &RED;
                out->has_label = true;
                out->position_offset = 2;
            }
            else
            {
                snprintf(out->value, sizeof(out->value), "NO DATA");
            }
            break;

        case METRIC_TOTAL_VALUE:
            if (all_invalid)
            {
                snprintf(out->value, sizeof(out->value), "NO DATA");
            }
            else
            {
                snprintf(out->label, sizeof(out->label), "  TOTAL");
                snprintf(out->value, sizeof(out->value), "%s%.2f %+.2f%%", currency_symbol,
                         total_value, total_pct);
                if (total_pct != 0.0f) out->color = (total_pct > 0) ? &GREEN : &RED;
                out->has_label = true;
                out->position_offset = 1;
            }
            break;

        default:
            break;
    }
}

static int portfolio_metric_width(const MetricRenderData* m)
{
    int label_w = m->has_label ? matrix_calculate_word_width_with_space(m->label) : 0;
    int value_w = matrix_calculate_word_width_with_space(m->value);
    int w = label_w + value_w;
    if (m->has_label)
    {
        w -= (m->position_offset == -1) ? 2 : 1;
    }
    return w;
}

static void portfolio_display_auto(Matrix* mtrx)
{
    float total_pl, total_pct, total_value;
    int winner_idx, loser_idx;
    portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx, &total_value);

    device_config_t config;
    flash_memory_read(&config);
    const char* currency_symbol = currency_get_symbol(config.currency);
    bool daily_mode = (config.portfolio_mode == 1);

    MetricRenderData metrics[METRIC_COUNT];
    int metric_widths[METRIC_COUNT];
    int total_width = 0;

    for (int i = 0; i < METRIC_COUNT; i++)
    {
        portfolio_build_metric_data(i, &metrics[i], total_pl, total_pct, total_value, winner_idx,
                                    loser_idx, currency_symbol, daily_mode);
        int w = portfolio_metric_width(&metrics[i]) + 12;
        metric_widths[i] = w;
        total_width += w;
    }

    if (total_width == 0) return;

    matrix_clear(mtrx);

    int current_x = 0;
    for (int i = 0; i < METRIC_COUNT; i++)
    {
        const MetricRenderData* m = &metrics[i];
        int label_w = m->has_label ? matrix_calculate_word_width_with_space(m->label) : 0;
        int base_x = current_x - g_auto_scroll_offset;

        for (int wrap = 0; wrap < 2; wrap++)
        {
            int x = base_x + (wrap * total_width) + 2;

            if (m->has_label)
            {
                matrix_display_word(m->label, x, 1, &LABEL_ORANGE);
                matrix_display_word(m->value, x + label_w + m->position_offset, 1, m->color);
            }
            else
            {
                matrix_display_word(m->value, x, 1, m->color);
            }
        }

        current_x += metric_widths[i];
    }

    matrix_show(mtrx);

    g_auto_scroll_offset += 2;
    if (g_auto_scroll_offset >= total_width)
    {
        g_auto_scroll_offset = 0;
    }
}

static void portfolio_display_metric(Matrix* mtrx, PortfolioMetric metric)
{
    float total_pl, total_pct, total_value;
    int winner_idx, loser_idx;
    portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx, &total_value);

    device_config_t config;
    flash_memory_read(&config);
    const char* currency_symbol = currency_get_symbol(config.currency);
    bool daily_mode = (config.portfolio_mode == 1);

    MetricRenderData m;
    portfolio_build_metric_data(metric, &m, total_pl, total_pct, total_value, winner_idx, loser_idx,
                                currency_symbol, daily_mode);

    int label_width = m.has_label ? matrix_calculate_word_width_with_space(m.label) : 0;
    int value_width = matrix_calculate_word_width_with_space(m.value);
    int total_width = label_width + value_width;

    if (m.has_label)
    {
        total_width -= (m.position_offset == -1) ? 2 : 1;
    }

    matrix_clear(mtrx);

    if (total_width >= 31)
    {
        for (int wrap = 0; wrap < 2; wrap++)
        {
            int base_x = (wrap * (total_width + 4)) - g_scroll_offset;
            if (m.has_label)
            {
                matrix_display_word(m.label, base_x, 1, &LABEL_ORANGE);
                matrix_display_word(m.value, base_x + label_width + m.position_offset, 1, m.color);
            }
            else
            {
                matrix_display_word(m.value, base_x, 1, m.color);
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
        if (m.has_label)
        {
            matrix_display_word(m.label, x_pos, 1, &LABEL_ORANGE);
            matrix_display_word(m.value, x_pos + label_width + m.position_offset, 1, m.color);
        }
        else
        {
            matrix_display_word(m.value, x_pos, 1, m.color);
        }
        matrix_show(mtrx);
    }
}

static void portfolio_display_url(Matrix* mtrx)
{
    const char* url_text = "    http://delta.local/portfolio";
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

    uint64_t current_time_ms = get_current_time_ms();
    bool is_first_fetch = (g_next_fetch_time_ms == 0);
    bool should_fetch = is_first_fetch || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        portfolio_load_from_flash();
        if (g_portfolio_count > 0)
        {
            portfolio_fetch_prices(mtrx, is_first_fetch);
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
            g_url_scroll_offset += 2;
            int url_width =
                matrix_calculate_word_width_with_space("    http://delta.local/portfolio") + 4;
            if (g_url_scroll_offset >= url_width)
            {
                g_url_scroll_offset = 0;
            }
        }
    }
    else if (g_data_fetched)
    {
        if (!g_scroll_type_loaded)
        {
            device_config_t scroll_config;
            flash_memory_read(&scroll_config);
            g_scroll_type = scroll_config.scroll_type;
            g_scroll_type_loaded = true;
        }

        if (g_scroll_type == 0)
        {
            portfolio_display_auto(mtrx);
        }
        else
        {
            portfolio_display_metric(mtrx, g_current_metric);
        }
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
    g_auto_scroll_offset = 0;

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
    g_auto_scroll_offset = 0;

    portfolio_display_metric(mtrx, g_current_metric);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_left(mtrx, current_state, next_state);
}

bool portfolio_is_auto_scroll(void)
{
    if (!g_scroll_type_loaded)
    {
        device_config_t config;
        flash_memory_read(&config);
        g_scroll_type = config.scroll_type;
        g_scroll_type_loaded = true;
    }
    return (g_scroll_type == 0);
}

void portfolio_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_scroll_offset = 0;
    g_url_scroll_offset = 0;
    g_current_metric = METRIC_PL_ABSOLUTE;
    g_auto_scroll_offset = 0;
    g_scroll_type_loaded = false;
    print(TRACE, "Portfolio cleanup called");
}
