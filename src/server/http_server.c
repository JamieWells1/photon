#include <http_server.h>

#include <const.h>
#include <flash_memory.h>
#include <macro.h>
#include <matrix.h>

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

static const char* html_form =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<meta charset=\"UTF-8\"><title>Photon Setup</title>"
    "<style>"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe "
    "UI',Roboto,sans-serif;background:linear-gradient(135deg,#4A90E2 0%,#357ABD "
    "100%);margin:0;padding:20px;min-height:100vh;display:flex;align-items:center;justify-content:"
    "center}"
    ".container{background:white;border-radius:12px;box-shadow:0 8px 24px "
    "rgba(0,0,0,0.15);padding:32px;max-width:400px;width:100%}"
    "h1{color:#333;margin:0 0 24px;font-size:28px;font-weight:600;text-align:center}"
    "label{display:block;color:#555;font-size:14px;font-weight:500;margin-bottom:8px}"
    "input{width:100%;padding:12px;border:2px solid "
    "#e0e0e0;border-radius:8px;font-size:16px;box-sizing:border-box;transition:border-color "
    "0.3s;margin-bottom:20px}"
    "input:focus{outline:none;border-color:#4A90E2}"
    "button{width:100%;background:linear-gradient(135deg,#4A90E2 0%,#357ABD "
    "100%);color:white;border:none;padding:14px;border-radius:8px;font-size:16px;font-weight:600;"
    "cursor:pointer;transition:transform 0.2s,box-shadow 0.2s}"
    "button:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(74,144,226,0.4)}"
    "button:active{transform:translateY(0)}"
    ".subtitle{color:#888;font-size:14px;text-align:center;margin-bottom:24px}"
    "</style></head><body>"
    "<div class=\"container\"><h1>Photon Setup</h1>"
    "<p class=\"subtitle\">Connect your device to WiFi</p>"
    "<form method=\"POST\" action=\"/save\" enctype=\"application/x-www-form-urlencoded\">"
    "<label>Network Name (SSID)</label><input type=\"text\" name=\"ssid\" placeholder=\"Enter WiFi "
    "network name\" required>"
    "<label>Password</label><input type=\"password\" name=\"password\" placeholder=\"Enter WiFi "
    "password\">"
    "<button type=\"submit\">Connect to WiFi</button></form></div></body></html>";

static const char* html_success =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<meta charset=\"UTF-8\"><title>Success</title>"
    "<style>"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe "
    "UI',Roboto,sans-serif;background:linear-gradient(135deg,#4A90E2 0%,#357ABD "
    "100%);margin:0;padding:20px;min-height:100vh;display:flex;align-items:center;justify-content:"
    "center}"
    ".container{background:white;border-radius:12px;box-shadow:0 8px 24px "
    "rgba(0,0,0,0.15);padding:32px;max-width:400px;width:100%;text-align:center}"
    "h1{color:#10b981;margin:0 0 16px;font-size:32px;font-weight:600}"
    "p{color:#555;font-size:16px;line-height:1.6;margin:0}"
    ".checkmark{width:64px;height:64px;margin:0 auto "
    "24px;border-radius:50%;background:#10b981;display:flex;align-items:center;justify-content:"
    "center;font-size:40px;color:white}"
    "</style></head><body>"
    "<div class=\"container\"><div class=\"checkmark\">✓</div><h1>Success!</h1>"
    "<p>Device is rebooting...<br>Do not close this page</p></div></body></html>";

static const char* html_config =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<meta charset=\"UTF-8\"><title>Photon Config</title>"
    "<style>"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:linear-gradient(135deg,#4A90E2 0%,#357ABD 100%);margin:0;padding:20px;min-height:100vh;display:flex;align-items:center;justify-content:center}"
    ".container{background:white;border-radius:12px;box-shadow:0 8px 24px rgba(0,0,0,0.15);padding:32px;max-width:400px;width:100%}"
    "h1{color:#333;margin:0 0 24px;font-size:28px;font-weight:600;text-align:center}"
    "label{display:block;color:#555;font-size:14px;font-weight:500;margin-bottom:8px;margin-top:16px}"
    "input,select{width:100%;padding:12px;border:2px solid #e0e0e0;border-radius:8px;font-size:16px;box-sizing:border-box;transition:border-color 0.3s}"
    "input:focus,select:focus{outline:none;border-color:#4A90E2}"
    "button{width:100%;background:linear-gradient(135deg,#4A90E2 0%,#357ABD 100%);color:white;border:none;padding:14px;border-radius:8px;font-size:16px;font-weight:600;cursor:pointer;transition:transform 0.2s,box-shadow 0.2s;margin-top:20px}"
    "button:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(74,144,226,0.4)}"
    "button:active{transform:translateY(0)}"
    ".subtitle{color:#888;font-size:14px;text-align:center;margin-bottom:24px}"
    ".range-value{display:inline-block;color:#4A90E2;font-weight:600;margin-left:8px}"
    ".warning{display:none;color:#f59e0b;font-size:13px;margin-top:4px;font-weight:500}"
    ".warning.show{display:block}"
    ".ticker-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:8px}"
    ".ticker-grid label{display:flex;align-items:center;gap:8px;margin:0;cursor:pointer}"
    ".ticker-grid input[type=checkbox]{width:auto;margin:0;padding:0}"
    "</style></head><body>"
    "<div class=\"container\"><h1>Device Settings</h1>"
    "<p class=\"subtitle\">Configure your Photon display</p>"
    "<form method=\"POST\" action=\"/save-config\" enctype=\"application/x-www-form-urlencoded\">"
    "<label>Brightness <span class=\"range-value\" id=\"brightnessValue\">2%</span></label>"
    "<input type=\"range\" name=\"brightness\" id=\"brightness\" min=\"1\" max=\"25\" value=\"2\" step=\"1\" oninput=\"updateBrightness(this.value)\">"
    "<p class=\"warning\" id=\"powerWarning\">⚠️ External power supply highly recommended to avoid overload</p>"
    "<label>Location</label>"
    "<select name=\"location\">"
    "<option value=\"0\">London</option>"
    "<option value=\"1\">New York</option>"
    "<option value=\"2\">Los Angeles</option>"
    "<option value=\"3\">Chicago</option>"
    "<option value=\"4\">Miami</option>"
    "<option value=\"5\">San Francisco</option>"
    "<option value=\"6\">Seattle</option>"
    "<option value=\"7\">Toronto</option>"
    "<option value=\"8\">Paris</option>"
    "<option value=\"9\">Berlin</option>"
    "<option value=\"10\">Stockholm</option>"
    "<option value=\"11\">Athens</option>"
    "<option value=\"12\">Dubai</option>"
    "<option value=\"13\">Singapore</option>"
    "<option value=\"14\">Tokyo</option>"
    "<option value=\"15\">Sydney</option>"
    "<option value=\"16\">Rio de Janeiro</option>"
    "<option value=\"17\">Cape Town</option>"
    "</select>"
    "<label>Currency</label>"
    "<select name=\"currency\">"
    "<option value=\"0\">GBP (£)</option>"
    "<option value=\"1\">USD ($)</option>"
    "<option value=\"2\">EUR (€)</option>"
    "<option value=\"3\">JPY (¥)</option>"
    "<option value=\"4\">CAD ($)</option>"
    "<option value=\"5\">AUD ($)</option>"
    "<option value=\"6\">CHF (Fr)</option>"
    "<option value=\"7\">CNY (¥)</option>"
    "<option value=\"8\">INR (₹)</option>"
    "<option value=\"9\">KRW (₩)</option>"
    "<option value=\"10\">BRL (R$)</option>"
    "<option value=\"11\">SGD ($)</option>"
    "</select>"
    "<label>Tickers</label>"
    "<div class=\"ticker-grid\">"
    "<label><input type=\"checkbox\" name=\"tkr_0\" value=\"1\" checked>XBT</label>"
    "<label><input type=\"checkbox\" name=\"tkr_1\" value=\"1\" checked>ETH</label>"
    "<label><input type=\"checkbox\" name=\"tkr_2\" value=\"1\" checked>SOL</label>"
    "<label><input type=\"checkbox\" name=\"tkr_3\" value=\"1\">ADA</label>"
    "<label><input type=\"checkbox\" name=\"tkr_4\" value=\"1\">DOT</label>"
    "<label><input type=\"checkbox\" name=\"tkr_5\" value=\"1\">LINK</label>"
    "<label><input type=\"checkbox\" name=\"tkr_6\" value=\"1\">AVAX</label>"
    "<label><input type=\"checkbox\" name=\"tkr_7\" value=\"1\">DOGE</label>"
    "<label><input type=\"checkbox\" name=\"tkr_8\" value=\"1\">XRP</label>"
    "<label><input type=\"checkbox\" name=\"tkr_9\" value=\"1\">LTC</label>"
    "</div>"
    "<button type=\"submit\">Save & Reboot</button>"
    "</form></div>"
    "<script>"
    "function updateBrightness(val){"
    "document.getElementById('brightnessValue').textContent=val+'%';"
    "var warning=document.getElementById('powerWarning');"
    "if(val>10){warning.classList.add('show');}else{warning.classList.remove('show');}"
    "}"
    "</script>"
    "</body></html>";

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

    if (strncmp(request, "POST /save-config", 17) == 0)
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

            uint16_t tickers = 0;
            for (int i = 0; i < 10; i++)
            {
                char key[8];
                snprintf(key, sizeof(key), "tkr_%d=", i);
                if (strstr(body, key)) tickers |= (1 << i);
            }
            config.tickers = tickers ? tickers : DEFAULT_TICKERS;

            debug(INFO, "Parsed config - Brightness: %d%%, Location: %d, Currency: %d, Tickers: 0x%04X",
                  config.brightness, config.location, config.currency, config.tickers);

            flash_memory_write(&config);
            matrix_set_brightness(config.brightness);
            config_saved = true;

            tcp_write(pcb, html_success, strlen(html_success), 0);
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

                tcp_write(pcb, html_success, strlen(html_success), 0);
                tcp_output(pcb);
                sleep_ms(500);
            }
            else
            {
                debug(WARN, "Empty SSID received");
                tcp_write(pcb, html_form, strlen(html_form), 0);
                tcp_output(pcb);
                sleep_ms(500);
            }
        }
    }
    else if (strncmp(request, "GET /config", 11) == 0 || strncmp(request, "GET / ", 6) == 0)
    {
        tcp_write(pcb, html_config, strlen(html_config), 0);
        tcp_output(pcb);
        tcp_sent(pcb, http_sent_callback);

        return ERR_OK;
    }
    else
    {
        tcp_write(pcb, html_form, strlen(html_form), 0);
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

    debug(INFO, "HTTP connection accepted from %s:%d",
          ipaddr_ntoa(&newpcb->remote_ip), newpcb->remote_port);

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
