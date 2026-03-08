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

typedef struct
{
    char buffer[2048];
    size_t received;
    size_t content_length;
    bool headers_complete;
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

    while (*line_start && config->portfolio_count < 30)
    {
        line_end = strchr(line_start, '\n');
        if (!line_end) line_end = line_start + strlen(line_start);

        // Skip header line and empty lines
        if (line_num > 0 && (line_end - line_start) > 0)
        {
            // Extract first column (ticker) - handle quoted CSV
            const char* ticker_start = line_start;
            const char* ticker_end = NULL;

            if (*ticker_start == '"')
            {
                ticker_start++;
                ticker_end = strchr(ticker_start, '"');
            }
            else
            {
                ticker_end = strchr(ticker_start, ',');
            }

            if (!ticker_end || ticker_end > line_end)
            {
                ticker_end = line_end;
            }

            size_t ticker_len = ticker_end - ticker_start;

            // Skip "Total" row and empty tickers
            if (ticker_len > 0 && ticker_len < 10 && strncmp(ticker_start, "Total", 5) != 0)
            {
                strncpy(config->portfolio_tickers[config->portfolio_count], ticker_start,
                        ticker_len);
                config->portfolio_tickers[config->portfolio_count][ticker_len] = '\0';

                debug(DEBUG, "Parsed ticker %d: %s", config->portfolio_count,
                      config->portfolio_tickers[config->portfolio_count]);

                config->portfolio_count++;
            }
        }

        line_num++;
        line_start = (*line_end == '\n') ? line_end + 1 : line_end;
    }

    debug(INFO, "Parsed %d tickers from CSV", config->portfolio_count);
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
    if (arg)
    {
        free(arg);
    }
}

static err_t http_close_conn(struct tcp_pcb* pcb)
{
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);

    return tcp_close(pcb);
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

    if (strncmp(request, "POST /upload-portfolio", 22) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            // Extract boundary from Content-Type header
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
                // Find the CSV file content
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

                        // Allocate temporary buffer for CSV content
                        char* csv_data = (char*)malloc(csv_len + 1);
                        if (csv_data)
                        {
                            memcpy(csv_data, file_start, csv_len);
                            csv_data[csv_len] = '\0';

                            debug(DEBUG, "CSV data length: %zu bytes", csv_len);

                            device_config_t config;
                            flash_memory_read(&config);

                            if (parse_csv_portfolio(csv_data, &config))
                            {
                                flash_memory_write(&config);

                                const char* response = html_get_portfolio_success();
                                tcp_write(pcb, response, strlen(response), 0);
                                tcp_output(pcb);
                            }
                            else
                            {
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
            if (val) config.brightness = (uint8_t)atoi(val + 11);
            val = strstr(body, "location=");
            if (val) config.location = (uint8_t)atoi(val + 9);
            val = strstr(body, "currency=");
            if (val) config.currency = (uint8_t)atoi(val + 9);

            uint16_t crypto = 0;
            for (int i = 0; i < CRYPTO_COUNT; i++)
            {
                char key[12];
                snprintf(key, sizeof(key), "crypto_%d=", i);
                if (strstr(body, key)) crypto |= (1 << i);
            }
            config.crypto = crypto ? crypto : DEFAULT_CRYPTO;

            uint16_t stocks = 0;
            for (int i = 0; i < STK_COUNT; i++)
            {
                char key[8];
                snprintf(key, sizeof(key), "stk_%d=", i);
                if (strstr(body, key)) stocks |= (1 << i);
            }
            config.stocks = stocks ? stocks : DEFAULT_STOCKS;

            debug(INFO,
                  "Parsed config - Brightness: %d%%, Location: %d, Currency: %d, Crypto: 0x%04X, "
                  "Stocks: 0x%04X",
                  config.brightness, config.location, config.currency, config.crypto,
                  config.stocks);

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
    else if (strncmp(request, "GET /config", 11) == 0 || strncmp(request, "GET / ", 6) == 0)
    {
        device_config_t config;
        flash_memory_read(&config);
        const char* config_page = html_build_config_page(&config);
        tcp_write(pcb, config_page, strlen(config_page), TCP_WRITE_FLAG_COPY);
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
