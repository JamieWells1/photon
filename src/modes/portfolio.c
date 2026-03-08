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
    float current_price;
} PortfolioTicker;

static PortfolioTicker g_portfolio_tickers[30];
static uint8_t g_portfolio_count = 0;
static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_ticker_to_fetch_index = 0;
static int g_scroll_offset = 0;

// Display logic

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
        g_portfolio_tickers[g_ticker_to_fetch_index].current_price = current_price;
        g_data_fetched = true;

        uint64_t ms_until_next_fetch = 15 * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed %s price: $%.2f",
              g_portfolio_tickers[g_ticker_to_fetch_index].ticker, current_price);
        debug(TRACE, "Next fetch scheduled in 15 minutes");
    }
    else
    {
        debug(ERROR, "Failed to parse portfolio data for index %u", g_ticker_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
}

void portfolio_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    uint64_t current_time_ms = get_current_time_ms();
    bool should_fetch = (g_next_fetch_time_ms == 0) || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        device_config_t config;
        flash_memory_read(&config);

        if (config.portfolio_count > 0 && g_portfolio_count != config.portfolio_count)
        {
            g_portfolio_count = config.portfolio_count;
            for (uint8_t i = 0; i < g_portfolio_count; i++)
            {
                strncpy(g_portfolio_tickers[i].ticker, config.portfolio_tickers[i], 10);
                g_portfolio_tickers[i].current_price = 0.0f;
            }
            debug(INFO, "Loaded %d portfolio tickers", g_portfolio_count);
        }

        if (g_portfolio_count > 0 && wifi_is_connected())
        {
            debug(INFO, "Starting portfolio data fetch");

            while (g_ticker_to_fetch_index < g_portfolio_count)
            {
                const char* ticker = g_portfolio_tickers[g_ticker_to_fetch_index].ticker;

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
    }

    if (g_data_fetched && g_portfolio_count > 0)
    {
        float total_value = 0.0f;
        for (uint8_t i = 0; i < g_portfolio_count; i++)
        {
            total_value += g_portfolio_tickers[i].current_price;
        }

        matrix_clear(mtrx);

        char display_str[64];
        snprintf(display_str, sizeof(display_str), "PORTFOLIO: $%.2f", total_value);

        int text_width = matrix_calculate_word_width_with_space(display_str);
        int x_pos = 16 - text_width / 2 - *offset;

        matrix_display_word(display_str, x_pos, 1, &GREEN);
        matrix_show(mtrx);

        (*offset) += 1;
        if (*offset >= text_width + 32)
        {
            *offset = 0;
        }
    }
    else if (g_portfolio_count == 0)
    {
        matrix_clear(mtrx);
        matrix_display_word("UPLOAD CSV", 2, 1, &DEFAULT_COLOUR);
        matrix_show(mtrx);
    }
    else
    {
        matrix_clear(mtrx);
        matrix_display_word("LOADING", 4, 1, &DEFAULT_COLOUR);
        matrix_show(mtrx);
    }
}

void portfolio_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_ticker_to_fetch_index = 0;
    g_scroll_offset = 0;
    debug(TRACE, "Portfolio cleanup called");
}
