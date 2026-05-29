/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <http_server.h>
#include <parser.h>

#include <const.h>
#include <crypto.h>
#include <error.h>
#include <flash_memory.h>
#include <html.h>
#include <macro.h>
#include <matrix.h>
#include <portfolio.h>
#include <stock.h>
#include <update.h>

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
static bool update_triggered = false;

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

typedef struct
{
    const char* data;
    size_t total_len;
    size_t sent;
    bool should_close;
} http_send_state_t;

// Forward declarations
static err_t http_close_conn(struct tcp_pcb* pcb);

static void http_err_callback(void* arg, err_t err)
{
    print(WARN, "HTTP error callback: err=%d, arg=%p", err, arg);
    if (arg)
    {
        free(arg);
    }
}

static err_t http_continue_sending(struct tcp_pcb* pcb, http_send_state_t* send_state)
{
    const size_t max_chunk = 1024;

    while (send_state->sent < send_state->total_len)
    {
        u16_t available = tcp_sndbuf(pcb);

        print(TRACE, "Continue send: offset %zu/%zu, available buffer = %u bytes", send_state->sent,
              send_state->total_len, available);

        if (available < 256)
        {
            print(TRACE, "Waiting for more buffer space (available: %u)", available);
            tcp_output(pcb);
            return ERR_OK;  // Will continue in sent callback
        }

        size_t to_send = send_state->total_len - send_state->sent;
        if (to_send > max_chunk) to_send = max_chunk;
        if (to_send > available) to_send = available;

        err_t err =
            tcp_write(pcb, send_state->data + send_state->sent, to_send, TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK)
        {
            print(ERROR, "tcp_write failed at offset %zu: %d", send_state->sent, err);
            return err;
        }

        send_state->sent += to_send;
        tcp_output(pcb);
    }

    print(DEBUG, "Send complete: %zu bytes", send_state->sent);
    return ERR_OK;
}

static err_t http_sent_callback_with_continuation(void* arg, struct tcp_pcb* pcb, u16_t len)
{
    http_send_state_t* send_state = (http_send_state_t*)arg;

    print(TRACE, "Sent callback: %u bytes ACKed, %zu/%zu total sent", len, send_state->sent,
          send_state->total_len);

    if (send_state->sent < send_state->total_len)
    {
        // Continue sending remaining data
        err_t err = http_continue_sending(pcb, send_state);
        if (err != ERR_OK)
        {
            print(ERROR, "Continue sending failed: %d", err);
            free(send_state);
            tcp_arg(pcb, NULL);
            http_close_conn(pcb);
            return err;
        }
    }
    else
    {
        // All data sent, clean up and close if needed
        print(DEBUG, "All data transmitted and ACKed");
        bool should_close = send_state->should_close;
        free(send_state);
        tcp_arg(pcb, NULL);

        if (should_close)
        {
            http_close_conn(pcb);
        }
    }

    return ERR_OK;
}

static err_t http_send_data(struct tcp_pcb* pcb, const char* data, size_t len,
                            http_request_state_t* state)
{
    print(DEBUG, "Sending %zu bytes with continuation support", len);

    // Allocate send state for continuation
    http_send_state_t* send_state = (http_send_state_t*)malloc(sizeof(http_send_state_t));
    if (!send_state)
    {
        print(ERROR, "Failed to allocate send state");
        return ERR_MEM;
    }

    send_state->data = data;
    send_state->total_len = len;
    send_state->sent = 0;
    send_state->should_close = true;

    // Set up sent callback for continuation
    tcp_arg(pcb, send_state);
    tcp_sent(pcb, http_sent_callback_with_continuation);

    // Start sending
    err_t err = http_continue_sending(pcb, send_state);
    if (err != ERR_OK)
    {
        free(send_state);
        tcp_arg(pcb, NULL);
        return err;
    }

    return ERR_OK;
}

static err_t http_close_conn(struct tcp_pcb* pcb)
{
    print(TRACE, "Closing HTTP connection, pcb=%p, state=%d, unsent=%p, unacked=%p", pcb,
          pcb->state, pcb->unsent, pcb->unacked);

    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);

    err_t err = tcp_close(pcb);
    if (err != ERR_OK)
    {
        print(WARN, "tcp_close failed: %d, calling tcp_abort", err);
        tcp_abort(pcb);
        return ERR_ABRT;
    }
    print(TRACE, "tcp_close succeeded");
    return ERR_OK;
}

static err_t http_sent_callback_simple(void* arg, struct tcp_pcb* pcb, u16_t len)
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
    print(TRACE, "http_recv_callback: pcb=%p, pbuf=%p, pbuf_len=%u, err=%d", pcb, p,
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

            print(TRACE, "Headers complete. Content-Length: %zu", state->content_length);
        }
        else
        {
            return ERR_OK;
        }
    }

    // Check if we have received the complete body (must be checked on every recv)
    if (state->headers_complete && state->content_length > 0)
    {
        char* body_start = strstr(state->buffer, "\r\n\r\n");
        if (body_start)
        {
            size_t header_len = (body_start + 4) - state->buffer;
            size_t body_received = state->received - header_len;

            print(TRACE, "Body received: %zu / %zu", body_received, state->content_length);

            if (body_received < state->content_length)
            {
                return ERR_OK;  // Wait for more body data
            }
        }
    }

    char* request = state->buffer;

    print(TRACE, "HTTP request complete, length: %zu", state->received);
    print(DEBUG, "Request first line: %.100s", request);

    // Prevent processing the same request multiple times
    if (state->request_processed)
    {
        print(DEBUG, "Request already processed, ignoring");
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
                // Find the portfolio file part specifically by its Content-Disposition name
                const char* portfolio_part = strstr(body, "name=\"portfolio\"");
                const char* file_start = portfolio_part ? strstr(portfolio_part, "\r\n\r\n") : NULL;
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

                            print(DEBUG, "CSV data length: %zu bytes", csv_len);

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
                                    config.brightness =
                                        brightness ? brightness : DEFAULT_BRIGHTNESS;
                                    print(DEBUG, "Parsed brightness: %d", config.brightness);
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
                                    print(DEBUG, "Parsed location: %d", config.location);
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
                                    print(DEBUG, "Parsed currency: %d", config.currency);
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
                                    print(DEBUG, "Parsed portfolio_mode: %d",
                                          config.portfolio_mode);
                                }
                            }

                            // Parse scroll_type
                            field = strstr(body, "name=\"scroll_type\"");
                            if (field)
                            {
                                val = strstr(field, "\r\n\r\n");
                                if (val)
                                {
                                    val += 4;
                                    config.scroll_type = (uint8_t)atoi(val);
                                    print(DEBUG, "Parsed scroll_type: %d", config.scroll_type);
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

                            print(
                                INFO,
                                "Upload - Brightness: %d%%, Location: %d, Currency: %d, Mode: %d, "
                                "Crypto: 0x%04X",
                                config.brightness, config.location, config.currency,
                                config.portfolio_mode, config.crypto);

                            ReturnCode rc = parser_csv_portfolio(csv_data, &config);
                            if (rc == RC_OK)
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
                                stock_invalidate_cache();
                                portfolio_invalidate_cache();
                                portfolio_parse_failed = false;

                                const char* response = html_get_portfolio_success();
                                tcp_write(pcb, response, strlen(response), 0);
                                tcp_output(pcb);
                            }
                            else
                            {
                                portfolio_parse_failed = true;
                                print(WARN, "CSV parse failed: %s", rc_to_string(rc));

                                const char* response = html_get_portfolio_error(rc_to_string(rc));
                                tcp_write(pcb, response, strlen(response), 0);
                                tcp_output(pcb);
                            }

                            free(csv_data);
                        }
                    }
                }
            }

            tcp_sent(pcb, http_sent_callback_simple);
            return ERR_OK;
        }
    }
    else if (strncmp(request, "POST /save-config", 17) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            print(DEBUG, "Config POST body: %s", body);

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
            val = strstr(body, "scroll_type=");
            if (val) config.scroll_type = (uint8_t)atoi(val + 12);

            uint16_t crypto = 0;
            for (int i = 0; i < CRYPTO_COUNT; i++)
            {
                char key[12];
                snprintf(key, sizeof(key), "c%d=", i);
                if (strstr(body, key)) crypto |= (1 << i);
            }
            config.crypto = crypto ? crypto : DEFAULT_CRYPTO;

            // Parse stock checkboxes (sN=on) for portfolio users
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

            // Always parse manual tickers (mt_N=TICKER)
            config.stock_ticker_count = 0;
            for (int i = 0; i < 10; i++)
            {
                char key[8];
                snprintf(key, sizeof(key), "mt_%d=", i);
                const char* ticker_start = strstr(body, key);
                if (ticker_start)
                {
                    ticker_start += strlen(key);
                    const char* ticker_end = ticker_start;
                    while (*ticker_end && *ticker_end != '&' && *ticker_end != '\r' &&
                           (ticker_end - ticker_start) < 9)
                    {
                        ticker_end++;
                    }
                    size_t len = ticker_end - ticker_start;
                    if (len > 0 && len < 10)
                    {
                        strncpy(config.stock_tickers[config.stock_ticker_count], ticker_start, len);
                        config.stock_tickers[config.stock_ticker_count][len] = '\0';
                        config.stock_ticker_count++;
                        print(DEBUG, "Parsed manual ticker %d: %s", config.stock_ticker_count - 1,
                              config.stock_tickers[config.stock_ticker_count - 1]);
                    }
                }
            }

            // If no portfolio, enable all manual tickers
            if (config.portfolio_count == 0 && config.stock_ticker_count > 0)
            {
                config.stocks = (1ULL << config.stock_ticker_count) - 1;
            }
            print(INFO, "Parsed %d manual tickers", config.stock_ticker_count);

            // Parse currency pairs (cp_N=PAIR)
            config.currency_pair_count = 0;
            for (int i = 0; i < 10; i++)
            {
                char key[8];
                snprintf(key, sizeof(key), "cp_%d=", i);
                const char* pair_start = strstr(body, key);
                if (pair_start)
                {
                    pair_start += strlen(key);
                    const char* pair_end = pair_start;
                    while (*pair_end && *pair_end != '&' && *pair_end != '\r' &&
                           (pair_end - pair_start) < 6)
                    {
                        pair_end++;
                    }
                    size_t len = pair_end - pair_start;
                    if (len == 6)
                    {
                        strncpy(config.currency_pairs[config.currency_pair_count], pair_start, 6);
                        config.currency_pairs[config.currency_pair_count][6] = '\0';
                        config.currency_pair_count++;
                        print(DEBUG, "Parsed currency pair %d: %s", config.currency_pair_count - 1,
                              config.currency_pairs[config.currency_pair_count - 1]);
                    }
                }
            }
            print(INFO, "Parsed %d currency pairs", config.currency_pair_count);

            print(INFO,
                  "Parsed config - Brightness: %d%%, Location: %d, Currency: %d, Crypto: 0x%04X, "
                  "Stocks: 0x%llX, Stock count: %d",
                  config.brightness, config.location, config.currency, config.crypto,
                  (unsigned long long)config.stocks, config.stock_ticker_count);

            flash_memory_write(&config);
            matrix_set_brightness(config.brightness);
            config_saved = true;

            tcp_write(pcb, html_get_success_page(), strlen(html_get_success_page()), 0);
            tcp_output(pcb);
            tcp_sent(pcb, http_sent_callback_simple);

            return ERR_OK;
        }
    }
    else if (strncmp(request, "POST /save", 10) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            print(DEBUG, "POST body: %s", body);

            device_config_t config;
            parser_post_data(body, &config);

            config.brightness = DEFAULT_BRIGHTNESS;
            config.location = DEFAULT_LOCATION;
            config.currency = DEFAULT_CURRENCY;
            config.portfolio_mode = 0;

            print(DEBUG, "Parsed - SSID: '%s', Password: '%s'", config.ssid, config.password);
            print(DEBUG, "Set defaults - Brightness: %d, Location: %d, Currency: %d",
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
                print(WARN, "Empty SSID received");
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

        print(DEBUG, "Sending device page: %zu bytes", page_len);

        // Free request state before sending (send will use its own state)
        if (state)
        {
            free(state);
        }

        err_t err = http_send_data(pcb, page, page_len, NULL);
        if (err != ERR_OK)
        {
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        // http_send_data sets up its own sent callback
        return ERR_OK;
    }
    else if (strncmp(request, "GET /watchlist", 14) == 0 ||
             strncmp(request, "GET /tickers", 12) == 0)
    {
        device_config_t config;
        flash_memory_read(&config);
        const char* page = html_build_config_page(&config, "watchlist");
        size_t page_len = strlen(page);

        print(DEBUG, "Sending watchlist page: %zu bytes", page_len);

        // Free request state before sending (send will use its own state)
        if (state)
        {
            free(state);
        }

        err_t err = http_send_data(pcb, page, page_len, NULL);
        if (err != ERR_OK)
        {
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        // http_send_data sets up its own sent callback
        return ERR_OK;
    }
    else if (strncmp(request, "GET /portfolio", 14) == 0 ||
             strncmp(request, "GET /upload ", 12) == 0)
    {
        device_config_t config;
        flash_memory_read(&config);
        const char* page = html_build_config_page(&config, "portfolio");
        size_t page_len = strlen(page);

        print(DEBUG, "Sending portfolio page: %zu bytes", page_len);

        // Free request state before sending (send will use its own state)
        if (state)
        {
            free(state);
        }

        err_t err = http_send_data(pcb, page, page_len, NULL);
        if (err != ERR_OK)
        {
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        // http_send_data sets up its own sent callback
        print(DEBUG, "Portfolio page send initiated");
        return ERR_OK;
    }
    else if (strncmp(request, "GET /update", 11) == 0)
    {
        bool update_avail = update_is_available();
        const char* remote_ver = update_get_remote_version();
        const char* page = html_build_update_page(update_avail, remote_ver);
        size_t page_len = strlen(page);

        print(DEBUG, "Sending update page: %zu bytes", page_len);

        if (state)
        {
            free(state);
        }

        err_t err = http_send_data(pcb, page, page_len, NULL);
        if (err != ERR_OK)
        {
            http_close_conn(pcb);
            return ERR_ABRT;
        }

        return ERR_OK;
    }
    else if (strncmp(request, "POST /update", 12) == 0)
    {
        print(INFO, "Update endpoint triggered - entering bootloader mode");
        update_triggered = true;

        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "Entering bootloader mode...";

        tcp_write(pcb, response, strlen(response), 0);
        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback_simple);

        return ERR_OK;
    }
    else
    {
        tcp_write(pcb, html_get_wifi_form(), strlen(html_get_wifi_form()), 0);
        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback_simple);

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
        print(WARN, "HTTP accept failed: err=%d", err);
        return ERR_VAL;
    }

    print(INFO, "HTTP connection accepted from %s:%d", ipaddr_ntoa(&newpcb->remote_ip),
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
    update_triggered = false;
    g_validation_state = 0;
    g_pending_ticker[0] = '\0';

    http_pcb = tcp_new();
    if (!http_pcb)
    {
        print(ERROR, "Failed to create HTTP PCB");
        return;
    }

    err_t err = tcp_bind(http_pcb, IP_ADDR_ANY, 80);
    if (err != ERR_OK)
    {
        print(ERROR, "Failed to bind HTTP server: %d", err);
        tcp_close(http_pcb);
        http_pcb = NULL;
        return;
    }

    http_pcb = tcp_listen(http_pcb);
    tcp_accept(http_pcb, http_accept_callback);

    print(INFO, "HTTP server started on port 80");
}

void http_server_stop(void)
{
    if (http_pcb)
    {
        tcp_close(http_pcb);
        http_pcb = NULL;
        print(INFO, "HTTP server stopped");
    }
}

bool http_server_credentials_saved(void) { return credentials_saved; }

bool http_server_config_saved(void) { return config_saved; }

bool http_server_portfolio_parse_failed(void) { return portfolio_parse_failed; }

bool http_server_update_triggered(void) { return update_triggered; }

bool http_server_has_pending_validation(void) { return g_validation_state == 1; }

const char* http_server_get_pending_ticker(void) { return g_pending_ticker; }

void http_server_set_validation_result(bool valid) { g_validation_state = valid ? 2 : 3; }
