#include <http.h>

#include <glyphs.h>
#include <graphics.h>
#include <macro.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lwip/dns.h>
#include <lwip/tcp.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>
#include "const.h"

#define HTTP_RESPONSE_BUFFER_SIZE 4096
#define HTTP_TIMEOUT_MS 30000

typedef struct
{
    struct tcp_pcb* pcb;
    char* request_path;
    http_response_callback_t callback;
    char response_buffer[HTTP_RESPONSE_BUFFER_SIZE];
    size_t bytes_received;
    bool headers_complete;
    char* body_start;
    bool complete;
    bool error;
} http_state_t;

static http_state_t* g_http_state = NULL;

static void http_cleanup(http_state_t* state)
{
    if (!state) return;

    if (state->pcb)
    {
        tcp_arg(state->pcb, NULL);
        tcp_recv(state->pcb, NULL);
        tcp_err(state->pcb, NULL);
        tcp_close(state->pcb);
        state->pcb = NULL;
    }

    if (state->request_path)
    {
        free(state->request_path);
        state->request_path = NULL;
    }

    free(state);
    g_http_state = NULL;
}

static err_t http_recv_callback(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
    http_state_t* state = (http_state_t*)arg;

    if (!p)
    {
        debug("HTTP connection closed by remote host");
        if (state->callback && state->body_start)
        {
            char* buffer_end = state->response_buffer + state->bytes_received;
            if (state->body_start >= state->response_buffer && state->body_start <= buffer_end)
            {
                state->callback(state->body_start, strlen(state->body_start), true);
            }
        }
        state->complete = true;
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(p);
        state->error = true;
        state->complete = true;
        return err;
    }

    size_t space_left = HTTP_RESPONSE_BUFFER_SIZE - state->bytes_received - 1;
    size_t copy_len = p->tot_len < space_left ? p->tot_len : space_left;

    if (copy_len > 0)
    {
        pbuf_copy_partial(p, state->response_buffer + state->bytes_received, copy_len, 0);
        state->bytes_received += copy_len;
        state->response_buffer[state->bytes_received] = '\0';
    }

    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    if (!state->headers_complete)
    {
        char* header_end = strstr(state->response_buffer, "\r\n\r\n");
        if (header_end)
        {
            state->headers_complete = true;
            state->body_start = header_end + 4;

            char* buffer_end = state->response_buffer + state->bytes_received;
            if (state->body_start < state->response_buffer || state->body_start > buffer_end)
            {
                state->body_start = buffer_end;
            }

            if (state->callback && state->body_start && state->body_start < buffer_end)
            {
                state->callback(state->body_start, strlen(state->body_start), false);
            }
        }
    }
    else
    {
        if (state->callback)
        {
            state->callback(state->response_buffer + state->bytes_received - copy_len, copy_len,
                            false);
        }
    }

    if (state->bytes_received >= HTTP_RESPONSE_BUFFER_SIZE - 100)
    {
        debug("HTTP response buffer nearly full, closing connection");
        if (state->callback && state->body_start)
        {
            state->callback(state->body_start, strlen(state->body_start), true);
        }
        state->complete = true;
        return ERR_OK;
    }

    return ERR_OK;
}

static err_t http_connected_callback(void* arg, struct tcp_pcb* pcb, err_t err)
{
    http_state_t* state = (http_state_t*)arg;

    if (err != ERR_OK)
    {
        debug("HTTP connection failed: %d", err);
        state->error = true;
        state->complete = true;
        return err;
    }

    debug("HTTP connected, sending request");

    char request[512];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             state->request_path, HOST_URL_WEATHER);

    err_t write_err = tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK)
    {
        debug("HTTP write failed: %d", write_err);
        state->error = true;
        state->complete = true;
        return write_err;
    }

    err_t output_err = tcp_output(pcb);
    if (output_err != ERR_OK)
    {
        debug("HTTP output failed: %d", output_err);
        state->error = true;
        state->complete = true;
        return output_err;
    }

    return ERR_OK;
}

static void http_error_callback(void* arg, err_t err)
{
    http_state_t* state = (http_state_t*)arg;
    debug("HTTP error: %d", err);

    if (state)
    {
        state->error = true;
        state->complete = true;
    }
}

static void http_dns_callback(const char* hostname, const ip_addr_t* ipaddr, void* arg)
{
    http_state_t* state = (http_state_t*)arg;

    if (!ipaddr)
    {
        debug("DNS lookup failed for %s", hostname);
        state->error = true;
        state->complete = true;
        return;
    }

    debug("DNS resolved: %s", ipaddr_ntoa(ipaddr));

    state->pcb = tcp_new();
    if (!state->pcb)
    {
        debug("Failed to create TCP PCB");
        state->error = true;
        state->complete = true;
        return;
    }

    tcp_arg(state->pcb, state);
    tcp_recv(state->pcb, http_recv_callback);
    tcp_err(state->pcb, http_error_callback);

    err_t err = tcp_connect(state->pcb, ipaddr, 80, http_connected_callback);
    if (err != ERR_OK)
    {
        debug("TCP connect failed: %d", err);
        state->error = true;
        state->complete = true;
        return;
    }

    debug("TCP connection initiated");
}

static void http_display_status(Matrix* mtrx, int dot_counter)
{
    char status[256] = "DATA";

    // Append animated dots
    size_t len = strlen(status);
    int num_dots = (dot_counter % 3) + 1;
    for (int i = 0; i < num_dots && len + i + 1 < sizeof(status); i++)
    {
        status[len + i] = '.';
        status[len + i + 1] = '\0';
    }

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(status, &DEFAULT_COLOUR, &ICONS_ARR[DATA], 0);
    matrix_show(mtrx);
}

int http_get(const char* host_url, const char* url_sub_path, http_response_callback_t callback,
             Matrix* mtrx)
{
    if (!host_url || !url_sub_path || !callback)
    {
        debug("Invalid parameters to http_get");
        return -1;
    }

    if (g_http_state)
    {
        debug("HTTP request already in progress");
        return -1;
    }

    g_http_state = (http_state_t*)calloc(1, sizeof(http_state_t));
    if (!g_http_state)
    {
        debug("Failed to allocate HTTP state");
        return -1;
    }

    g_http_state->request_path = strdup(url_sub_path);
    g_http_state->callback = callback;
    g_http_state->headers_complete = false;
    g_http_state->bytes_received = 0;
    g_http_state->body_start = NULL;
    g_http_state->complete = false;
    g_http_state->error = false;

    debug("Starting DNS lookup for %s", host_url);

    ip_addr_t resolved_addr;
    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(host_url, &resolved_addr, http_dns_callback, g_http_state);
    cyw43_arch_lwip_end();

    if (err == ERR_OK)
    {
        debug("DNS cached");
        http_dns_callback(host_url, &resolved_addr, g_http_state);
    }
    else if (err != ERR_INPROGRESS)
    {
        debug("DNS lookup failed immediately: %d", err);
        http_cleanup(g_http_state);
        return -1;
    }

    // Block until complete or timeout
    uint64_t start_time = to_ms_since_boot(get_absolute_time());
    int dot_counter = 0;
    int display_frame_counter = 0;

    while (!g_http_state->complete)
    {
        // Update display every 3-4 polls (300-400ms between frames)
        if (display_frame_counter % 4 == 0)
        {
            http_display_status(mtrx, dot_counter);
            dot_counter++;
        }
        display_frame_counter++;

        // Check timeout
        uint64_t elapsed = to_ms_since_boot(get_absolute_time()) - start_time;
        if (elapsed > HTTP_TIMEOUT_MS)
        {
            debug("HTTP request timeout");
            http_cleanup(g_http_state);

            // Display timeout
            matrix_clear(mtrx);
            matrix_display_word_icon_pair("TIMEOUT", &RED, NULL, 0);
            matrix_show(mtrx);
            sleep_ms(1000);

            return -1;
        }

        // Allow lwIP to process
        cyw43_arch_poll();
        sleep_ms(100);
    }

    bool had_error = g_http_state->error;
    http_cleanup(g_http_state);

    if (had_error)
    {
        debug("HTTP request failed");

        // Display timeout
        matrix_clear(mtrx);
        matrix_display_word_icon_pair("TIMEOUT", &RED, NULL, 0);
        matrix_show(mtrx);
        sleep_ms(1000);

        return -1;
    }

    debug("HTTP request completed successfully");

    // Brief delay to show DATA status before transitioning to weather display
    sleep_ms(300);

    return 0;
}
