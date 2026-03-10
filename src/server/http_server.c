/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <http_server.h>

#include <const.h>
#include <crypto.h>
#include <flash_memory.h>
#include <html.h>
#include <macro.h>
#include <matrix.h>
#include <stock.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/watchdog.h>
#include <lwip/tcp.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>

static struct tcp_pcb* http_pcb = NULL;
static bool credentials_saved = false;
static bool config_saved = false;
static bool portfolio_parse_failed = false;

// Ticker validation state: 0=idle, 1=pending, 2=valid, 3=invalid
static int8_t g_validation_state = 0;
static char g_pending_ticker[10] = {0};

typedef struct
{
    char buffer[8192];
    size_t received;
    size_t content_length;
    bool headers_complete;
    bool request_processed;
} http_request_state_t;

static int hex_to_int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

static void url_decode(char* dst, const char* src, size_t max_len)
{
    size_t i = 0;
    size_t j = 0;

    while (src[i] && j < max_len - 1)
    {
        if (src[i] == '%' && src[i + 1] && src[i + 2])
        {
            dst[j++] = (hex_to_int(src[i + 1]) << 4) | hex_to_int(src[i + 2]);
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

static bool parse_csv_portfolio(const char* csv_data, device_config_t* config)
{
    const char* line_start = csv_data;
    const char* line_end;
    int line_num = 0;
    config->portfolio_count = 0;

    debug(DEBUG, "Starting CSV parse, data preview: %.100s", csv_data);

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
        debug(TRACE, "Line %d, length %zu: %.*s", line_num, line_length, (int)line_length,
              line_start);

        if (line_num > 0 && line_length > 0)
        {
            const char* col_start = line_start;
            const char* col_end = NULL;
            int col_num = 0;
            char ticker[10] = {0};
            float invested_value = 0.0f;
            float current_value = 0.0f;
            float quantity = 0.0f;

            while (col_start < line_end_trimmed && col_num < 6)
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
                debug(TRACE, "  Col %d, len %zu: '%.*s'", col_num, col_len, (int)col_len,
                      col_start);

                if (col_num == 0 && col_len > 0 && col_len < 10 &&
                    strncmp(col_start, "Total", 5) != 0)
                {
                    strncpy(ticker, col_start, col_len);
                    ticker[col_len] = '\0';
                }
                else if (col_num == 2)
                {
                    invested_value = atof(col_start);
                }
                else if (col_num == 3)
                {
                    current_value = atof(col_start);
                }
                else if (col_num == 5)
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

                debug(DEBUG,
                      "✓ Parsed position %d: %s qty=%.4f avg_price=%.2f csv_price=%.2f "
                      "invested=%.2f value=%.2f",
                      config->portfolio_count, ticker, quantity, avg_price, csv_price,
                      invested_value, current_value);

                config->portfolio_count++;
            }
            else
            {
                debug(TRACE, "✗ Skipped line %d: ticker='%s' invested=%.2f qty=%.2f (invalid)",
                      line_num, ticker, invested_value, quantity);
            }
        }

        line_num++;
        line_start = (*line_end == '\n') ? line_end + 1 : line_end;
    }

    debug(INFO, "Parsed %d positions from CSV", config->portfolio_count);
    return config->portfolio_count > 0;
}

static void parse_post_data(const char* body, device_config_t* config)
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

        url_decode(config->ssid, temp_ssid, sizeof(config->ssid));
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

        url_decode(config->password, temp_password, sizeof(config->password));
    }

    debug(INFO, "Parsed WiFi credentials - SSID: %s", config->ssid);
}

static void http_err_callback(void* arg, err_t err)
{
    debug(WARN, "HTTP error callback: err=%d, arg=%p", err, arg);
    if (arg)
    {
        free(arg);
    }
}

static err_t http_send_data(struct tcp_pcb* pcb, const char* data, size_t len,
                            http_request_state_t* state)
{
    debug(DEBUG, "Sending %zu bytes", len);

    if (len <= 2048)
    {
        err_t err = tcp_write(pcb, data, len, 0);
        if (err != ERR_OK)
        {
            debug(ERROR, "tcp_write failed: %d", err);
            return err;
        }
        return ERR_OK;
    }

    const size_t chunk_size = 2048;
    size_t sent = 0;

    while (sent < len)
    {
        size_t to_send = (len - sent) > chunk_size ? chunk_size : (len - sent);
        err_t err = tcp_write(pcb, data + sent, to_send, TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK)
        {
            debug(ERROR, "tcp_write failed at offset %zu: %d", sent, err);
            return err;
        }
        sent += to_send;
    }

    err_t err = tcp_output(pcb);
    if (err != ERR_OK)
    {
        debug(ERROR, "tcp_output failed: %d", err);
        return err;
    }

    debug(DEBUG, "Successfully sent %zu bytes in chunks", sent);
    return ERR_OK;
}

static err_t http_close_conn(struct tcp_pcb* pcb)
{
    debug(TRACE, "Closing HTTP connection, pcb=%p, state=%d, unsent=%p, unacked=%p", pcb,
          pcb->state, pcb->unsent, pcb->unacked);

    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);

    err_t err = tcp_close(pcb);
    if (err != ERR_OK)
    {
        debug(WARN, "tcp_close failed: %d, calling tcp_abort", err);
        tcp_abort(pcb);
        return ERR_ABRT;
    }
    debug(TRACE, "tcp_close succeeded");
    return ERR_OK;
}

static err_t http_sent_callback(void* arg, struct tcp_pcb* pcb, u16_t len)
{
    if (arg)
    {
        free(arg);
    }

    http_close_conn(pcb);
    return ERR_OK;
}

static err_t http_recv_callback(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
    debug(TRACE, "http_recv_callback: pcb=%p, pbuf=%p, pbuf_len=%u, err=%d", pcb, p,
          p ? p->tot_len : 0, err);

    if (!p)
    {
        if (arg)
        {
            free(arg);
            tcp_arg(pcb, NULL);
        }
        http_close_conn(pcb);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(p);
        return err;
    }

    http_request_state_t* state = (http_request_state_t*)arg;
    if (!state)
    {
        state = (http_request_state_t*)calloc(1, sizeof(http_request_state_t));
        if (!state)
        {
            pbuf_free(p);
            http_close_conn(pcb);
            return ERR_MEM;
        }
        tcp_arg(pcb, state);
    }

    size_t len = p->tot_len;
    if (state->received + len > sizeof(state->buffer) - 1)
    {
        len = sizeof(state->buffer) - 1 - state->received;
    }

    pbuf_copy_partial(p, state->buffer + state->received, len, 0);
    state->received += len;
    state->buffer[state->received] = '\0';

    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    if (!state->headers_complete)
    {
        char* body_start = strstr(state->buffer, "\r\n\r\n");
        if (body_start)
        {
            state->headers_complete = true;
            char* cl_header = strstr(state->buffer, "Content-Length:");
            if (cl_header)
            {
                state->content_length = atoi(cl_header + 15);
            }

            size_t header_len = (body_start + 4) - state->buffer;
            size_t body_received = state->received - header_len;

            debug(TRACE, "Headers complete. Content-Length: %zu, Body received: %zu",
                  state->content_length, body_received);

            if (body_received < state->content_length)
            {
                return ERR_OK;
            }
        }
        else
        {
            return ERR_OK;
        }
    }

    char* request = state->buffer;

    debug(TRACE, "HTTP request complete, length: %zu", state->received);
    debug(DEBUG, "Request first line: %.100s", request);

    // Prevent processing the same request multiple times
    if (state->request_processed)
    {
        debug(DEBUG, "Request already processed, ignoring");
        return ERR_OK;
    }
    state->request_processed = true;

    if (strncmp(request, "POST /upload", 12) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            char boundary[128] = {0};
            const char* ct_header = strstr(request, "Content-Type:");
            if (ct_header)
            {
                const char* boundary_start = strstr(ct_header, "boundary=");
                if (boundary_start)
                {
                    boundary_start += 9;
                    const char* boundary_end = strstr(boundary_start, "\r\n");
                    size_t boundary_len = boundary_end ? (size_t)(boundary_end - boundary_start)
                                                       : strlen(boundary_start);
                    if (boundary_len < sizeof(boundary))
                    {
                        strncpy(boundary, boundary_start, boundary_len);
                        boundary[boundary_len] = '\0';
                    }
                }
            }

            if (strlen(boundary) > 0)
            {
                const char* file_start = strstr(body, "\r\n\r\n");
                if (file_start)
                {
                    file_start += 4;
                    char end_marker[140];
                    snprintf(end_marker, sizeof(end_marker), "\r\n--%s", boundary);
                    const char* file_end = strstr(file_start, end_marker);

                    if (file_end)
                    {
                        size_t csv_len = file_end - file_start;

                        char* csv_data = (char*)malloc(csv_len + 1);
                        if (csv_data)
                        {
                            memcpy(csv_data, file_start, csv_len);
                            csv_data[csv_len] = '\0';

                            debug(DEBUG, "CSV data length: %zu bytes", csv_len);

                            device_config_t config;
                            flash_memory_read(&config);

                            // Parse all settings from multipart form data
                            const char* field;
                            const char* val;

                            // Parse brightness
                            field = strstr(body, "name=\"brightness\"");
                            if (field)
                            {
                                val = strstr(field, "\r\n\r\n");
                                if (val)
                                {
                                    val += 4;
                                    uint8_t brightness = (uint8_t)atoi(val);
                                    config.brightness = brightness ? brightness : DEFAULT_BRIGHTNESS;
                                    debug(DEBUG, "Parsed brightness: %d", config.brightness);
                                }
                            }

                            // Parse location
                            field = strstr(body, "name=\"location\"");
                            if (field)
                            {
                                val = strstr(field, "\r\n\r\n");
                                if (val)
                                {
                                    val += 4;
                                    config.location = (uint8_t)atoi(val);
                                    debug(DEBUG, "Parsed location: %d", config.location);
                                }
                            }

                            // Parse currency
                            field = strstr(body, "name=\"currency\"");
                            if (field)
                            {
                                val = strstr(field, "\r\n\r\n");
                                if (val)
                                {
                                    val += 4;
                                    config.currency = (uint8_t)atoi(val);
                                    debug(DEBUG, "Parsed currency: %d", config.currency);
                                }
                            }

                            // Parse portfolio_mode
                            field = strstr(body, "name=\"portfolio_mode\"");
                            if (field)
                            {
                                val = strstr(field, "\r\n\r\n");
                                if (val)
                                {
                                    val += 4;
                                    config.portfolio_mode = (uint8_t)atoi(val);
                                    debug(DEBUG, "Parsed portfolio_mode: %d", config.portfolio_mode);
                                }
                            }

                            // Parse crypto checkboxes
                            uint16_t crypto = 0;
                            for (int i = 0; i < CRYPTO_COUNT; i++)
                            {
                                char key[20];
                                snprintf(key, sizeof(key), "name=\"c%d\"", i);
                                if (strstr(body, key)) crypto |= (1 << i);
                            }
                            if (crypto) config.crypto = crypto;

                            debug(INFO,
                                  "Upload - Brightness: %d%%, Location: %d, Currency: %d, Mode: %d, "
                                  "Crypto: 0x%04X",
                                  config.brightness, config.location, config.currency,
                                  config.portfolio_mode, config.crypto);

                            if (parse_csv_portfolio(csv_data, &config))
                            {
                                // Default first 10 portfolio tickers enabled
                                {
                                    int default_count =
                                        config.portfolio_count < 10 ? config.portfolio_count : 10;
                                    config.stocks =
                                        default_count > 0 ? ((1ULL << default_count) - 1) : 0;
                                }
                                // Clear manual tickers since portfolio is the source
                                config.stock_ticker_count = 0;
                                flash_memory_write(&config);
                                portfolio_parse_failed = false;

                                const char* response = html_get_portfolio_success();
                                tcp_write(pcb, response, strlen(response), 0);
                                tcp_output(pcb);
                            }
                            else
                            {
                                portfolio_parse_failed = true;

                                const char* response = html_get_portfolio_error();
                                tcp_write(pcb, response, strlen(response), 0);
                                tcp_output(pcb);
                            }

                            free(csv_data);
                        }
                    }
                }
            }

            tcp_sent(pcb, http_sent_callback);
            return ERR_OK;
        }
    }
    else if (strncmp(request, "POST /save-config", 17) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            debug(DEBUG, "Config POST body: %s", body);

            device_config_t config;
            flash_memory_read(&config);

            const char* val;
            val = strstr(body, "brightness=");
            if (val)
            {
                uint8_t brightness = (uint8_t)atoi(val + 11);
                config.brightness = brightness ? brightness : DEFAULT_BRIGHTNESS;
            }
            val = strstr(body, "location=");
            if (val) config.location = (uint8_t)atoi(val + 9);
            val = strstr(body, "currency=");
            if (val) config.currency = (uint8_t)atoi(val + 9);
            val = strstr(body, "portfolio_mode=");
            if (val) config.portfolio_mode = (uint8_t)atoi(val + 15);

            uint16_t crypto = 0;
            for (int i = 0; i < CRYPTO_COUNT; i++)
            {
                char key[12];
                snprintf(key, sizeof(key), "c%d=", i);
                if (strstr(body, key)) crypto |= (1 << i);
            }
            config.crypto = crypto ? crypto : DEFAULT_CRYPTO;

            // Parse stock checkboxes (sN=on) for portfolio users
            // Manual tickers are managed via /add-ticker and /remove-ticker endpoints
            if (config.portfolio_count > 0)
            {
                uint64_t stocks = 0;
                for (int i = 0; i < config.portfolio_count && i < 50; i++)
                {
                    char key[12];
                    snprintf(key, sizeof(key), "s%d=", i);
                    if (strstr(body, key)) stocks |= (1ULL << i);
                }
                config.stocks = stocks;
            }

            debug(INFO,
                  "Parsed config - Brightness: %d%%, Location: %d, Currency: %d, Crypto: 0x%04X, "
                  "Stocks: 0x%llX",
                  config.brightness, config.location, config.currency, config.crypto,
                  (unsigned long long)config.stocks);

            flash_memory_write(&config);
            matrix_set_brightness(config.brightness);
            config_saved = true;

            tcp_write(pcb, html_get_success_page(), strlen(html_get_success_page()), 0);
            tcp_output(pcb);
            tcp_sent(pcb, http_sent_callback);

            return ERR_OK;
        }
    }
    else if (strncmp(request, "POST /save", 10) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            debug(DEBUG, "POST body: %s", body);

            device_config_t config;
            parse_post_data(body, &config);

            config.brightness = DEFAULT_BRIGHTNESS;
            config.location = DEFAULT_LOCATION;
            config.currency = DEFAULT_CURRENCY;
            config.portfolio_mode = 0;

            debug(DEBUG, "Parsed - SSID: '%s', Password: '%s'", config.ssid, config.password);
            debug(DEBUG, "Set defaults - Brightness: %d, Location: %d, Currency: %d",
                  config.brightness, config.location, config.currency);

            if (strlen(config.ssid) > 0)
            {
                flash_memory_write(&config);
                credentials_saved = true;

                tcp_write(pcb, html_get_success_page(), strlen(html_get_success_page()), 0);
                tcp_output(pcb);
                sleep_ms(500);
            }
            else
            {
                debug(WARN, "Empty SSID received");
                tcp_write(pcb, html_get_wifi_form(), strlen(html_get_wifi_form()), 0);
                tcp_output(pcb);
                sleep_ms(500);
            }
        }
    }
    else if (strncmp(request, "GET /device", 11) == 0 || strncmp(request, "GET /config", 11) == 0 ||
             strncmp(request, "GET / ", 6) == 0)
    {
        device_config_t config;
        flash_memory_read(&config);
        const char* page = html_build_config_page(&config, "device");
        size_t page_len = strlen(page);

        debug(DEBUG, "Sending device page: %zu bytes", page_len);

        err_t err = http_send_data(pcb, page, page_len, state);
        if (err != ERR_OK)
        {
            if (state)
            {
                free(state);
                tcp_arg(pcb, NULL);
            }
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);

        return ERR_OK;
    }
    else if (strncmp(request, "GET /watchlist", 14) == 0 ||
             strncmp(request, "GET /tickers", 12) == 0)
    {
        device_config_t config;
        flash_memory_read(&config);
        const char* page = html_build_config_page(&config, "watchlist");
        size_t page_len = strlen(page);

        debug(DEBUG, "Sending watchlist page: %zu bytes", page_len);

        err_t err = http_send_data(pcb, page, page_len, state);
        if (err != ERR_OK)
        {
            if (state)
            {
                free(state);
                tcp_arg(pcb, NULL);
            }
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);

        return ERR_OK;
    }
    else if (strncmp(request, "GET /portfolio", 14) == 0 ||
             strncmp(request, "GET /upload ", 12) == 0)
    {
        device_config_t config;
        flash_memory_read(&config);
        const char* page = html_build_config_page(&config, "portfolio");
        size_t page_len = strlen(page);

        debug(DEBUG, "Sending portfolio page: %zu bytes", page_len);

        err_t err = http_send_data(pcb, page, page_len, state);
        if (err != ERR_OK)
        {
            if (state)
            {
                free(state);
                tcp_arg(pcb, NULL);
            }
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);

        debug(DEBUG, "Portfolio page sent successfully");
        return ERR_OK;
    }
    else if (strncmp(request, "GET /add-ticker", 15) == 0)
    {
        // Parse ticker from query string: /add-ticker?t=AAPL
        char ticker[10] = {0};
        const char* t_param = strstr(request, "?t=");
        if (t_param)
        {
            t_param += 3;
            const char* end = t_param;
            while (*end && *end != ' ' && *end != '&' && *end != '\r' && (end - t_param) < 9)
            {
                end++;
            }
            size_t len = end - t_param;
            if (len > 0 && len < 10)
            {
                strncpy(ticker, t_param, len);
                ticker[len] = '\0';
            }
        }

        if (ticker[0] != '\0')
        {
            // Check max limit
            device_config_t config;
            flash_memory_read(&config);
            if (config.stock_ticker_count >= 10)
            {
                const char* page = html_build_ticker_error_page("MAX 10 TICKERS");
                tcp_write(pcb, page, strlen(page), 0);
            }
            else
            {
                // Check for duplicate
                bool duplicate = false;
                for (int i = 0; i < config.stock_ticker_count; i++)
                {
                    if (strncmp(config.stock_tickers[i], ticker, 10) == 0)
                    {
                        duplicate = true;
                        break;
                    }
                }

                if (duplicate)
                {
                    const char* response =
                        "HTTP/1.1 302 Found\r\nLocation: /watchlist\r\nConnection: close\r\n\r\n";
                    tcp_write(pcb, response, strlen(response), 0);
                }
                else
                {
                    // Store pending ticker and start validation
                    strncpy(g_pending_ticker, ticker, sizeof(g_pending_ticker));
                    g_validation_state = 1;

                    debug(INFO, "Ticker validation requested: %s", ticker);

                    const char* page = html_build_ticker_checking_page(ticker);
                    tcp_write(pcb, page, strlen(page), 0);
                }
            }
        }
        else
        {
            const char* response =
                "HTTP/1.1 302 Found\r\nLocation: /watchlist\r\nConnection: close\r\n\r\n";
            tcp_write(pcb, response, strlen(response), 0);
        }

        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);
        return ERR_OK;
    }
    else if (strncmp(request, "GET /ticker-result", 18) == 0)
    {
        if (g_validation_state == 2)
        {
            // Valid — save ticker to config and redirect to watchlist
            device_config_t config;
            flash_memory_read(&config);

            if (config.stock_ticker_count < 10)
            {
                strncpy(config.stock_tickers[config.stock_ticker_count], g_pending_ticker, 10);
                config.stock_ticker_count++;
                config.stocks = (1ULL << config.stock_ticker_count) - 1;
                flash_memory_write(&config);
                debug(INFO, "Ticker %s validated and saved (total: %d)", g_pending_ticker,
                      config.stock_ticker_count);
            }

            g_validation_state = 0;
            g_pending_ticker[0] = '\0';

            const char* response =
                "HTTP/1.1 302 Found\r\nLocation: /watchlist\r\nConnection: close\r\n\r\n";
            tcp_write(pcb, response, strlen(response), 0);
        }
        else if (g_validation_state == 3)
        {
            // Invalid
            const char* page = html_build_ticker_error_page(g_pending_ticker);
            tcp_write(pcb, page, strlen(page), 0);

            g_validation_state = 0;
            g_pending_ticker[0] = '\0';
        }
        else
        {
            // Still pending — tell browser to retry
            const char* page = html_get_ticker_still_checking();
            tcp_write(pcb, page, strlen(page), 0);
        }

        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);
        return ERR_OK;
    }
    else if (strncmp(request, "GET /remove-ticker", 18) == 0)
    {
        // Parse index from query string: /remove-ticker?i=2
        const char* i_param = strstr(request, "?i=");
        if (i_param)
        {
            int idx = atoi(i_param + 3);
            device_config_t config;
            flash_memory_read(&config);

            if (idx >= 0 && idx < config.stock_ticker_count)
            {
                debug(INFO, "Removing ticker %d: %s", idx, config.stock_tickers[idx]);

                // Shift remaining tickers down
                for (int i = idx; i < config.stock_ticker_count - 1; i++)
                {
                    strncpy(config.stock_tickers[i], config.stock_tickers[i + 1], 10);
                }
                config.stock_ticker_count--;
                config.stocks = config.stock_ticker_count > 0
                                    ? ((1ULL << config.stock_ticker_count) - 1)
                                    : 0;
                flash_memory_write(&config);
            }
        }

        const char* response =
            "HTTP/1.1 302 Found\r\nLocation: /watchlist\r\nConnection: close\r\n\r\n";
        tcp_write(pcb, response, strlen(response), 0);
        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);
        return ERR_OK;
    }
    else
    {
        tcp_write(pcb, html_get_wifi_form(), strlen(html_get_wifi_form()), 0);
        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);

        return ERR_OK;
    }

    if (state)
    {
        free(state);
        tcp_arg(pcb, NULL);
    }

    http_close_conn(pcb);
    return ERR_OK;
}

static err_t http_accept_callback(void* arg, struct tcp_pcb* newpcb, err_t err)
{
    if (err != ERR_OK || !newpcb)
    {
        debug(WARN, "HTTP accept failed: err=%d", err);
        return ERR_VAL;
    }

    debug(INFO, "HTTP connection accepted from %s:%d", ipaddr_ntoa(&newpcb->remote_ip),
          newpcb->remote_port);

    tcp_recv(newpcb, http_recv_callback);
    tcp_err(newpcb, http_err_callback);

    return ERR_OK;
}

void http_server_start(void)
{
    if (http_pcb) return;

    credentials_saved = false;
    config_saved = false;
    portfolio_parse_failed = false;
    g_validation_state = 0;
    g_pending_ticker[0] = '\0';

    http_pcb = tcp_new();
    if (!http_pcb)
    {
        debug(ERROR, "Failed to create HTTP PCB");
        return;
    }

    err_t err = tcp_bind(http_pcb, IP_ADDR_ANY, 80);
    if (err != ERR_OK)
    {
        debug(ERROR, "Failed to bind HTTP server: %d", err);
        tcp_close(http_pcb);
        http_pcb = NULL;
        return;
    }

    http_pcb = tcp_listen(http_pcb);
    tcp_accept(http_pcb, http_accept_callback);

    debug(INFO, "HTTP server started on port 80");
}

void http_server_stop(void)
{
    if (http_pcb)
    {
        tcp_close(http_pcb);
        http_pcb = NULL;
        debug(INFO, "HTTP server stopped");
    }
}

bool http_server_credentials_saved(void) { return credentials_saved; }

bool http_server_config_saved(void) { return config_saved; }

bool http_server_portfolio_parse_failed(void) { return portfolio_parse_failed; }

bool http_server_has_pending_validation(void) { return g_validation_state == 1; }

const char* http_server_get_pending_ticker(void) { return g_pending_ticker; }

void http_server_set_validation_result(bool valid) { g_validation_state = valid ? 2 : 3; }
