#include <http.h>

#include <macro.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lwip/dns.h>
#include <lwip/tcp.h>
#include <pico/cyw43_arch.h>

#define HTTP_RESPONSE_BUFFER_SIZE 4096

typedef struct
{
    struct tcp_pcb* pcb;
    char* request_path;
    http_response_callback callback;
    char response_buffer[HTTP_RESPONSE_BUFFER_SIZE];
    size_t bytes_received;
    bool headers_complete;
    char* body_start;
} http_state_t;

static http_state_t* g_http_state = NULL;

static err_t http_recv_callback(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err);
static err_t http_connected_callback(void* arg, struct tcp_pcb* pcb, err_t err);
static void http_error_callback(void* arg, err_t err);
static void http_dns_callback(const char* hostname, const ip_addr_t* ipaddr, void* arg);
static void http_cleanup(http_state_t* state);

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
        http_cleanup(state);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(p);
        http_cleanup(state);
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
            state->body_start = header_end + 4;  // Skip past "\r\n\r\n"

            // Ensure body_start is within buffer bounds
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
        http_cleanup(state);
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
        http_cleanup(state);
        return err;
    }

    debug("HTTP connected, sending request");

    char request[512];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             state->request_path, "api.open-meteo.com");

    err_t write_err = tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    if (write_err != ERR_OK)
    {
        debug("HTTP write failed: %d", write_err);
        http_cleanup(state);
        return write_err;
    }

    err_t output_err = tcp_output(pcb);
    if (output_err != ERR_OK)
    {
        debug("HTTP output failed: %d", output_err);
        http_cleanup(state);
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
        http_cleanup(state);
    }
}

static void http_dns_callback(const char* hostname, const ip_addr_t* ipaddr, void* arg)
{
    http_state_t* state = (http_state_t*)arg;

    if (!ipaddr)
    {
        debug("DNS lookup failed for %s", hostname);
        http_cleanup(state);
        return;
    }

    debug("DNS resolved: %s", ipaddr_ntoa(ipaddr));

    state->pcb = tcp_new();
    if (!state->pcb)
    {
        debug("Failed to create TCP PCB");
        http_cleanup(state);
        return;
    }

    tcp_arg(state->pcb, state);
    tcp_recv(state->pcb, http_recv_callback);
    tcp_err(state->pcb, http_error_callback);

    err_t err = tcp_connect(state->pcb, ipaddr, 80, http_connected_callback);
    if (err != ERR_OK)
    {
        debug("TCP connect failed: %d", err);
        http_cleanup(state);
        return;
    }

    debug("TCP connection initiated");
}

int http_get(const char* host_url, const char* url_sub_path, http_response_callback callback)
{
    if (!host_url || !url_sub_path || !callback)
    {
        debug("Invalid parameters to http_get");
        return -1;
    }

    if (g_http_state)
    {
        debug("HTTP request already in progress");
        return -2;
    }

    g_http_state = (http_state_t*)calloc(1, sizeof(http_state_t));
    if (!g_http_state)
    {
        debug("Failed to allocate HTTP state");
        return -3;
    }

    g_http_state->request_path = strdup(url_sub_path);
    g_http_state->callback = callback;
    g_http_state->headers_complete = false;
    g_http_state->bytes_received = 0;
    g_http_state->body_start = NULL;

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
        return -4;
    }

    return 0;
}
