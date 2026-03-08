#include <stock.h>

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

Stock STK_ARR[STK_COUNT] = {
    {AAPL, STK_AAPL, 0.0f, 0.0f}, {MSFT, STK_MSFT, 0.0f, 0.0f}, {GOOGL, STK_GOOGL, 0.0f, 0.0f},
    {AMZN, STK_AMZN, 0.0f, 0.0f}, {TSLA, STK_TSLA, 0.0f, 0.0f}, {NVDA, STK_NVDA, 0.0f, 0.0f},
    {META, STK_META, 0.0f, 0.0f}, {JPM, STK_JPM, 0.0f, 0.0f},   {V, STK_V, 0.0f, 0.0f},
    {DIS, STK_DIS, 0.0f, 0.0f},
};

static volatile bool g_data_fetched = false;
static volatile uint64_t g_next_fetch_time_ms = 0;
static unsigned int g_stk_to_fetch_index = 0;
static int g_scroll_offset = 0;
static uint16_t g_enabled_mask = DEFAULT_STOCKS;

bool stock_is_enabled(StockIndex idx) { return (g_enabled_mask & (1 << idx)) != 0; }

void stock_set_enabled_mask(uint16_t mask)
{
    if (mask == 0) mask = DEFAULT_STOCKS;
    g_enabled_mask = mask;
}

// Display logic

static void stock_response_callback(const char* body, size_t len, bool complete)
{
    debug(TRACE, "stock_response_callback: len=%d, complete=%d", len, complete);

    if (!complete) return;

    if (!body || len == 0)
    {
        debug(ERROR, "Empty response for stock index %u", g_stk_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
        return;
    }

    debug(DEBUG, "Received stock response: %.*s", (int)len, body);

    float current_price = parse_price_from_json_array(body, "\"c\":");
    float open_price = parse_price_from_json_string(body, "\"o\":");

    if (!current_price) current_price = 0.0f;
    if (!open_price) open_price = 0.0f;

    if (current_price > 0.0f && open_price > 0.0f)
    {
        STK_ARR[g_stk_to_fetch_index].current_price = current_price;
        STK_ARR[g_stk_to_fetch_index].open_price = open_price;
        g_data_fetched = true;

        uint64_t ms_until_next_fetch = 15 * 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_next_fetch;

        debug(INFO, "Parsed current: $%.2f, open: $%.2f", current_price, open_price);
        debug(TRACE, "Next fetch scheduled in 15 minutes");
    }
    else
    {
        debug(ERROR, "Failed to parse stock data for index %u", g_stk_to_fetch_index);
        uint64_t ms_until_retry = 60 * 1000;
        g_next_fetch_time_ms = get_current_time_ms() + ms_until_retry;
    }
}

void stock_display(Matrix* mtrx, int* offset)
{
    ASSERT_NOT_NULL(mtrx);
    ASSERT_NOT_NULL(offset);

    uint64_t current_time_ms = get_current_time_ms();
    bool should_fetch = (g_next_fetch_time_ms == 0) || (current_time_ms >= g_next_fetch_time_ms);

    if (should_fetch)
    {
        debug(INFO, "Starting stock data fetch");

        if (wifi_is_connected())
        {
            device_config_t config;
            flash_memory_read(&config);
            stock_set_enabled_mask(config.stocks);

            while (g_stk_to_fetch_index < STK_COUNT)
            {
                if (!stock_is_enabled(g_stk_to_fetch_index))
                {
                    g_stk_to_fetch_index++;
                    continue;
                }

                const char* stk_to_fetch = STK_ARR[g_stk_to_fetch_index].to_str;

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
        int stock_widths[STK_COUNT];
        memset(stock_widths, 0, sizeof(stock_widths));

        for (unsigned int i = 0; i < STK_COUNT; i++)
        {
            if (!stock_is_enabled(i))
            {
                continue;
            }

            const Stock* stk = &STK_ARR[i];
            float current = stk->current_price;
            float open = stk->open_price;

            float change_perc = (open > 0.0f) ? ((current - open) / open) * 100 : 0.0f;

            char price_str[16];
            snprintf(price_str, sizeof(price_str), "%.2f", current);

            char change_perc_str[16];
            snprintf(change_perc_str, sizeof(change_perc_str), "%+.1f%%", change_perc);

            int width = matrix_calculate_word_width_with_space(stk->to_str) + 3 +
                        matrix_calculate_word_width_with_space(price_str) +
                        ICONS_ARR[TKR_UP].width +
                        matrix_calculate_word_width_with_space(change_perc_str) + 12;

            stock_widths[i] = width;
            total_width += width;
        }

        matrix_clear(mtrx);

        int current_x = 0;
        for (unsigned int i = 0; i < STK_COUNT; i++)
        {
            if (!stock_is_enabled(i))
            {
                continue;
            }

            const Stock* stk = &STK_ARR[i];

            float current = stk->current_price;
            float open = stk->open_price;

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
                int stk_x = base_x + (wrap * total_width) + 2;
                int price_x = stk_x + matrix_calculate_word_width_with_space(stk->to_str) + 3;
                int icon_x = price_x + matrix_calculate_word_width_with_space(price_str);
                int perc_diff_x = icon_x + icon->width + 4;

                const unsigned int y = 1;

                matrix_display_word(stk->to_str, stk_x, y, &LABEL_ORANGE);
                matrix_display_word(price_str, price_x, y, colour);
                matrix_display_icon(icon, icon_x, y, colour);
                matrix_display_word(change_perc_str, perc_diff_x, y, colour);
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

void stock_cleanup(void)
{
    g_data_fetched = false;
    g_next_fetch_time_ms = 0;
    g_stk_to_fetch_index = 0;
    g_scroll_offset = 0;
}
