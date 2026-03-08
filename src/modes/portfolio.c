#include <portfolio.h>

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

typedef struct
{
    char ticker[10];
    float quantity;
    float avg_price;
    float current_price;
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

static PortfolioPosition g_positions[30];
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

    float current_price = parse_price_from_json_array(body, "\"c\":");

    if (!current_price) current_price = 0.0f;

    if (current_price > 0.0f)
    {
        g_positions[g_ticker_to_fetch_index].current_price = current_price;
        g_data_fetched = true;

        uint64_t ms_until_next_fetch = 15 * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed %s price: $%.2f", g_positions[g_ticker_to_fetch_index].ticker,
              current_price);
        debug(TRACE, "Next fetch scheduled in 15 minutes");
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

    debug(INFO, "Starting portfolio data fetch");

    while (g_ticker_to_fetch_index < g_portfolio_count)
    {
        const char* ticker = g_positions[g_ticker_to_fetch_index].ticker;

        char path[256];
        snprintf(path, sizeof(path), "/stocks/%s", ticker);

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
            g_positions[i].current_price = 0.0f;
        }
        debug(INFO, "Loaded %d portfolio positions", g_portfolio_count);
    }
}

static void portfolio_calculate_metrics(float* total_pl, float* total_pct, int* winner_idx,
                                         int* loser_idx, float* total_value)
{
    float total_cost = 0.0f;
    *total_value = 0.0f;
    *winner_idx = -1;
    *loser_idx = -1;
    float max_pl = -999999.0f;
    float min_pl = 999999.0f;

    for (uint8_t i = 0; i < g_portfolio_count; i++)
    {
        if (g_positions[i].current_price == 0.0f) continue;

        float cost = g_positions[i].quantity * g_positions[i].avg_price;
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

    char display_str[64];
    const RGB* color = &DEFAULT_COLOUR;

    switch (metric)
    {
        case METRIC_PL_ABSOLUTE:
            snprintf(display_str, sizeof(display_str), "P/L $%.2f", total_pl);
            color = (total_pl >= 0) ? &GREEN : &RED;
            break;

        case METRIC_PL_PERCENTAGE:
            snprintf(display_str, sizeof(display_str), "P/L %.2f%%", total_pct);
            color = (total_pct >= 0) ? &GREEN : &RED;
            break;

        case METRIC_BIGGEST_WINNER:
            if (winner_idx >= 0)
            {
                float pl = g_positions[winner_idx].quantity *
                           (g_positions[winner_idx].current_price - g_positions[winner_idx].avg_price);
                snprintf(display_str, sizeof(display_str), "%s +$%.2f",
                         g_positions[winner_idx].ticker, pl);
                color = &GREEN;
            }
            else
            {
                snprintf(display_str, sizeof(display_str), "NO DATA");
            }
            break;

        case METRIC_BIGGEST_LOSER:
            if (loser_idx >= 0)
            {
                float pl = g_positions[loser_idx].quantity *
                           (g_positions[loser_idx].current_price - g_positions[loser_idx].avg_price);
                snprintf(display_str, sizeof(display_str), "%s $%.2f",
                         g_positions[loser_idx].ticker, pl);
                color = &RED;
            }
            else
            {
                snprintf(display_str, sizeof(display_str), "NO DATA");
            }
            break;

        case METRIC_TOTAL_VALUE:
            snprintf(display_str, sizeof(display_str), "VAL $%.2f %+.2f%%", total_value, total_pct);
            color = (total_pct >= 0) ? &GREEN : &RED;
            break;

        default:
            return;
    }

    int text_width = matrix_calculate_word_width_with_space(display_str);
    matrix_clear(mtrx);

    if (text_width > 30)
    {
        for (int wrap = 0; wrap < 2; wrap++)
        {
            int x_pos = (wrap * (text_width + 4)) - g_scroll_offset;
            matrix_display_word(display_str, x_pos, 1, color);
        }
        matrix_show(mtrx);
    }
    else
    {
        int x_pos = 1;
        matrix_display_word(display_str, x_pos, 1, color);
        matrix_show(mtrx);
    }
}

static void portfolio_display_url(Matrix* mtrx)
{
    const char* url_text = "    http://photon.local/upload";
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
        portfolio_display_url(mtrx);
        g_url_scroll_offset++;
        int url_width = matrix_calculate_word_width_with_space("    http://photon.local/upload") + 4;
        if (g_url_scroll_offset >= url_width)
        {
            g_url_scroll_offset = 0;
        }
    }
    else if (g_data_fetched)
    {
        portfolio_display_metric(mtrx, g_current_metric);

        char display_str[64];
        switch (g_current_metric)
        {
            case METRIC_PL_ABSOLUTE: {
                float total_pl, total_pct, total_value;
                int winner_idx, loser_idx;
                portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx,
                                             &total_value);
                snprintf(display_str, sizeof(display_str), "P/L $%.2f", total_pl);
                break;
            }
            case METRIC_PL_PERCENTAGE: {
                float total_pl, total_pct, total_value;
                int winner_idx, loser_idx;
                portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx,
                                             &total_value);
                snprintf(display_str, sizeof(display_str), "P/L %.2f%%", total_pct);
                break;
            }
            case METRIC_BIGGEST_WINNER: {
                float total_pl, total_pct, total_value;
                int winner_idx, loser_idx;
                portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx,
                                             &total_value);
                if (winner_idx >= 0)
                {
                    float pl = g_positions[winner_idx].quantity *
                               (g_positions[winner_idx].current_price -
                                g_positions[winner_idx].avg_price);
                    snprintf(display_str, sizeof(display_str), "%s +$%.2f",
                             g_positions[winner_idx].ticker, pl);
                }
                else
                {
                    snprintf(display_str, sizeof(display_str), "NO DATA");
                }
                break;
            }
            case METRIC_BIGGEST_LOSER: {
                float total_pl, total_pct, total_value;
                int winner_idx, loser_idx;
                portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx,
                                             &total_value);
                if (loser_idx >= 0)
                {
                    float pl = g_positions[loser_idx].quantity *
                               (g_positions[loser_idx].current_price -
                                g_positions[loser_idx].avg_price);
                    snprintf(display_str, sizeof(display_str), "%s $%.2f",
                             g_positions[loser_idx].ticker, pl);
                }
                else
                {
                    snprintf(display_str, sizeof(display_str), "NO DATA");
                }
                break;
            }
            case METRIC_TOTAL_VALUE: {
                float total_pl, total_pct, total_value;
                int winner_idx, loser_idx;
                portfolio_calculate_metrics(&total_pl, &total_pct, &winner_idx, &loser_idx,
                                             &total_value);
                snprintf(display_str, sizeof(display_str), "VAL $%.2f %+.2f%%", total_value,
                         total_pct);
                break;
            }
            default:
                snprintf(display_str, sizeof(display_str), "ERROR");
                break;
        }

        int text_width = matrix_calculate_word_width_with_space(display_str);
        if (text_width > 30)
        {
            g_scroll_offset++;
            if (g_scroll_offset >= text_width + 4)
            {
                g_scroll_offset = 0;
            }
        }
    }
    else
    {
        matrix_clear(mtrx);
        matrix_display_word("LOADING", 4, 1, &DEFAULT_COLOUR);
        matrix_show(mtrx);
    }
}

void portfolio_next_metric(void)
{
    g_current_metric = (g_current_metric + 1) % METRIC_COUNT;
    g_scroll_offset = 0;
}

void portfolio_prev_metric(void)
{
    if (g_current_metric == 0)
    {
        g_current_metric = METRIC_COUNT - 1;
    }
    else
    {
        g_current_metric--;
    }
    g_scroll_offset = 0;
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
