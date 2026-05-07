/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <parser.h>

#include <const.h>
#include <macro.h>

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parser_hex_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

void parser_decode_url(char* dst, const char* src, size_t max_len)
{
    size_t i = 0;
    size_t j = 0;

    while (src[i] && j < max_len - 1)
    {
        if (src[i] == '%' && src[i + 1] && src[i + 2])
        {
            dst[j++] = (parser_hex_to_int(src[i + 1]) << 4) | parser_hex_to_int(src[i + 2]);
            i += 3;
        }
        else if (src[i] == '+')
        {
            dst[j++] = ' ';
            i++;
        }
        else
        {
            dst[j++] = src[i++];
        }
    }
    dst[j] = '\0';
}

static int count_columns(const char* line_start, const char* line_end)
{
    int count = 1;
    for (const char* p = line_start; p < line_end; p++)
    {
        if (*p == ',') count++;
    }
    return count;
}

static bool header_is_ibkr(const char* line_start, const char* line_end)
{
    for (const char* p = line_start; p <= line_end - 14; p++)
    {
        if (strncmp(p, "CostBasisMoney", 14) == 0) return true;
    }
    return false;
}

ReturnCode parser_csv_portfolio(const char* csv_data, device_config_t* config)
{
    const char* line_start = csv_data;
    const char* line_end;
    int line_num = 0;
    config->portfolio_count = 0;
    bool ibkr_header = false;
    uint8_t csv_format = 0;  // determined from header on line 0

    print(DEBUG, "Starting CSV parse (auto-detect), data preview: %.100s", csv_data);

    // T212 columns: 0=ticker, 2=invested, 3=current_value, 5=quantity
    // IBKR columns: 0=ticker, 1=quantity, 2=invested, 3=current_value
    int col_ticker = 0;
    int col_quantity = -1;
    int col_invested = 2;
    int col_current = 3;
    int col_quantity2 = 5;
    int col_max = 6;

    while (*line_start && config->portfolio_count < 50)
    {
        line_end = strchr(line_start, '\n');
        if (!line_end) line_end = line_start + strlen(line_start);

        const char* line_end_trimmed = line_end;
        if (line_end > line_start && *(line_end - 1) == '\r')
        {
            line_end_trimmed = line_end - 1;
        }

        size_t line_length = line_end_trimmed - line_start;
        print(TRACE, "Line %d, length %zu: %.*s", line_num, line_length, (int)line_length,
              line_start);

        if (line_num == 0 && line_length > 0)
        {
            ibkr_header = header_is_ibkr(line_start, line_end_trimmed);
            csv_format = ibkr_header ? 1 : 0;
            if (ibkr_header)
            {
                col_quantity = 1;
                col_invested = 2;
                col_current = 3;
                col_quantity2 = -1;
                col_max = 4;
            }
            print(DEBUG, "Detected format: %s", ibkr_header ? "IBKR" : "T212");
        }

        if (line_num == 1 && line_length > 0)
        {
            int ncols = count_columns(line_start, line_end_trimmed);
            print(DEBUG, "First data row: %d columns, format=%s", ncols,
                  ibkr_header ? "IBKR" : "T212");

            if (csv_format == 1)  // IBKR detected via header: must have exactly 4 columns
            {
                if (ncols != 4)
                {
                    print(WARN, "IBKR header found but column count is %d (expected 4)", ncols);
                    return RC_PARSER_TOO_FEW_COLUMNS;
                }
            }
            else  // T212 assumed: must have at least 6 columns
            {
                if (ncols < 6)
                {
                    print(WARN, "No IBKR header and only %d columns, unrecognised format", ncols);
                    return RC_PARSER_UNRECOGNIZED_FORMAT;
                }
            }
        }

        if (line_num > 0 && line_length > 0)
        {
            const char* col_start = line_start;
            const char* col_end = NULL;
            int col_num = 0;
            char ticker[10] = {0};
            float invested_value = 0.0f;
            float current_value = 0.0f;
            float quantity = 0.0f;

            while (col_start < line_end_trimmed && col_num < col_max)
            {
                if (*col_start == '"')
                {
                    col_start++;
                    col_end = strchr(col_start, '"');
                }
                else
                {
                    col_end = strchr(col_start, ',');
                }

                if (!col_end || col_end > line_end_trimmed) col_end = line_end_trimmed;

                size_t col_len = col_end - col_start;
                print(TRACE, "  Col %d, len %zu: '%.*s'", col_num, col_len, (int)col_len,
                      col_start);

                if (col_num == col_ticker && col_len > 0 && col_len < 10 &&
                    strncmp(col_start, "Total", 5) != 0)
                {
                    strncpy(ticker, col_start, col_len);
                    ticker[col_len] = '\0';
                }
                else if (col_num == col_invested)
                {
                    invested_value = atof(col_start);
                }
                else if (col_num == col_current)
                {
                    current_value = atof(col_start);
                }
                else if (col_quantity >= 0 && col_num == col_quantity)
                {
                    quantity = atof(col_start);
                }
                else if (col_quantity2 >= 0 && col_num == col_quantity2)
                {
                    quantity = atof(col_start);
                }

                col_start = (*col_end == '"') ? col_end + 1 : col_end;
                if (*col_start == ',') col_start++;
                col_num++;
            }

            if (ticker[0] != '\0' && invested_value > 0.0f && quantity > 0.0f)
            {
                strncpy(config->portfolio_tickers[config->portfolio_count], ticker, 10);

                float avg_price = invested_value / quantity;
                float csv_price = (current_value > 0.0f) ? (current_value / quantity) : 0.0f;

                config->portfolio_quantities[config->portfolio_count] = quantity;
                config->portfolio_avg_prices[config->portfolio_count] = avg_price;
                config->portfolio_csv_prices[config->portfolio_count] = csv_price;

                print(DEBUG,
                      "✓ Parsed position %d: %s qty=%.4f avg_price=%.2f csv_price=%.2f "
                      "invested=%.2f value=%.2f",
                      config->portfolio_count, ticker, quantity, avg_price, csv_price,
                      invested_value, current_value);

                config->portfolio_count++;
            }
            else
            {
                print(TRACE, "✗ Skipped line %d: ticker='%s' invested=%.2f qty=%.2f (invalid)",
                      line_num, ticker, invested_value, quantity);
            }
        }

        line_num++;
        line_start = (*line_end == '\n') ? line_end + 1 : line_end;
    }

    print(INFO, "Parsed %d positions from CSV", config->portfolio_count);
    if (config->portfolio_count == 0) return RC_PARSER_NO_POSITIONS;
    if (config->portfolio_count == 50 && *line_start != '\0') return RC_PARSER_TOO_MANY_TICKERS;
    return RC_OK;
}

void parser_post_data(const char* body, device_config_t* config)
{
    memset(config, 0, sizeof(device_config_t));

    char temp_ssid[128] = {0};
    char temp_password[128] = {0};

    const char* ssid_start = strstr(body, "ssid=");
    if (ssid_start)
    {
        ssid_start += 5;
        const char* ssid_end = strchr(ssid_start, '&');
        if (!ssid_end) ssid_end = body + strlen(body);

        size_t ssid_len = ssid_end - ssid_start;
        if (ssid_len > sizeof(temp_ssid) - 1) ssid_len = sizeof(temp_ssid) - 1;
        strncpy(temp_ssid, ssid_start, ssid_len);
        temp_ssid[ssid_len] = '\0';

        parser_decode_url(config->ssid, temp_ssid, sizeof(config->ssid));
    }

    const char* password_start = strstr(body, "password=");
    if (password_start)
    {
        password_start += 9;
        const char* password_end = strchr(password_start, '&');
        if (!password_end) password_end = body + strlen(body);

        size_t password_len = password_end - password_start;
        if (password_len > sizeof(temp_password) - 1) password_len = sizeof(temp_password) - 1;
        strncpy(temp_password, password_start, password_len);
        temp_password[password_len] = '\0';

        parser_decode_url(config->password, temp_password, sizeof(config->password));
    }

    print(INFO, "Parsed WiFi credentials - SSID: %s", config->ssid);
}
