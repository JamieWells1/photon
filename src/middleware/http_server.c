#include <http_server.h>

#include <const.h>
#include <flash_config.h>
#include <macro.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <lwip/tcp.h>
#include <pico/stdlib.h>

static struct tcp_pcb* http_pcb = NULL;
static bool credentials_saved = false;

static const char* html_form =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<title>Photon Setup</title></head><body><h1>WiFi Setup</h1>"
    "<form method=\"POST\" action=\"/save\">"
    "SSID: <input name=\"ssid\" required><br>"
    "Password: <input name=\"password\" type=\"password\"><br>"
    "<button type=\"submit\">Connect</button></form></body></html>";

static const char* html_success =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<title>Success</title></head><body><h1>Success!</h1>"
    "<p>WiFi credentials saved. Device will reboot...</p></body></html>";

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
        if (src[i] == '%' && src[i+1] && src[i+2])
        {
            dst[j++] = (hex_to_int(src[i+1]) << 4) | hex_to_int(src[i+2]);
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

static void parse_post_data(const char* body, wifi_config_t* config)
{
    memset(config, 0, sizeof(wifi_config_t));

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

static err_t http_close_conn(struct tcp_pcb* pcb)
{
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);

    return tcp_close(pcb);
}

static err_t http_recv_callback(void* arg, struct tcp_pcb* pcb, struct pbuf* p, err_t err)
{
    if (!p)
    {
        http_close_conn(pcb);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(p);
        return err;
    }

    char request[2048];
    size_t len = p->tot_len;
    if (len > sizeof(request) - 1) len = sizeof(request) - 1;

    pbuf_copy_partial(p, request, len, 0);
    request[len] = '\0';

    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    if (strncmp(request, "POST /save", 10) == 0)
    {
        const char* body = strstr(request, "\r\n\r\n");
        if (body)
        {
            body += 4;

            debug(DEBUG, "POST body: %s", body);

            wifi_config_t config;
            parse_post_data(body, &config);

            debug(DEBUG, "Parsed - SSID: '%s', Password: '%s'", config.ssid, config.password);

            if (strlen(config.ssid) > 0)
            {
                flash_config_write(&config);
                credentials_saved = true;

                tcp_write(pcb, html_success, strlen(html_success), TCP_WRITE_FLAG_COPY);
                tcp_output(pcb);
                sleep_ms(100);
            }
            else
            {
                debug(WARN, "Empty SSID received");
                tcp_write(pcb, html_form, strlen(html_form), TCP_WRITE_FLAG_COPY);
                tcp_output(pcb);
                sleep_ms(100);
            }
        }
    }
    else
    {
        tcp_write(pcb, html_form, strlen(html_form), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        sleep_ms(100);
    }

    http_close_conn(pcb);
    return ERR_OK;
}

static err_t http_accept_callback(void* arg, struct tcp_pcb* newpcb, err_t err)
{
    if (err != ERR_OK || !newpcb)
    {
        return ERR_VAL;
    }

    tcp_recv(newpcb, http_recv_callback);

    return ERR_OK;
}

void http_server_start(void)
{
    if (http_pcb) return;

    credentials_saved = false;

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

bool http_server_credentials_saved(void)
{
    return credentials_saved;
}
