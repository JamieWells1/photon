#include <html.h>

#include <const.h>
#include <crypto.h>
#include <currency.h>
#include <location.h>
#include <stock.h>

#include <stdio.h>
#include <string.h>

static const char* html_wifi_form =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<title>Photon Setup</title>"
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
    "<title>Success</title>"
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
    "<div class=\"container\"><h1>Save Successful!</h1>"
    "<p>Device is rebooting...<br>You can close this page</p></div></body></html>";

static const char* html_portfolio_success =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<title>Portfolio Uploaded</title>"
    "<style>"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe "
    "UI',Roboto,sans-serif;background:linear-gradient(135deg,#4A90E2 0%,#357ABD "
    "100%);margin:0;padding:20px;min-height:100vh;display:flex;align-items:center;justify-content:"
    "center}"
    ".container{background:white;border-radius:12px;box-shadow:0 8px 24px "
    "rgba(0,0,0,0.15);padding:32px;max-width:400px;width:100%;text-align:center}"
    "h1{color:#10b981;margin:0 0 16px;font-size:32px;font-weight:600}"
    "p{color:#555;font-size:18px;line-height:1.6;margin:0;font-weight:500}"
    "</style></head><body>"
    "<div class=\"container\"><h1>Upload Successful!</h1>"
    "<p>You can now view your portfolio on your device</p></div></body></html>";

static const char* html_portfolio_error =
    "HTTP/1.1 400 Bad Request\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n\r\n"
    "Failed to parse CSV";

static char config_html_buf[16384];

const char* html_get_wifi_form(void) { return html_wifi_form; }

const char* html_get_success_page(void) { return html_success; }

const char* html_get_portfolio_success(void) { return html_portfolio_success; }

const char* html_get_portfolio_error(void) { return html_portfolio_error; }

const char* html_build_config_page(const device_config_t* config, const char* active_tab)
{
    int pos = 0;

    // HTTP headers + minimal HTML/CSS start
    pos +=
        snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                 "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
                 "<!DOCTYPE html><html><head><meta name=viewport content=width=device-width>"
                 "<title>Photon</title><style>"
                 "body{background:#4A90E2;margin:0;padding:10px;font:14px sans-serif}"
                 ".c{background:#fff;padding:15px;max-width:400px;margin:0 auto}"
                 "h1{margin:0 0 10px;font-size:20px}"
                 ".tabs{display:flex;border-bottom:1px solid #ddd}"
                 ".tab{flex:1;padding:8px;text-align:center;cursor:pointer;border:none;background:"
                 "none;color:#888}"
                 ".tab.active{color:#4A90E2;border-bottom:2px solid #4A90E2}"
                 ".t{display:none}.t.active{display:block}"
                 "label{display:block;margin:10px 0 3px}"
                 "input,select{width:100%%;padding:6px;border:1px solid #ddd;box-sizing:border-box}"
                 "button{width:100%%;background:#4A90E2;color:#fff;border:none;padding:10px;cursor:"
                 "pointer;margin-top:10px}"
                 ".g{display:grid;grid-template-columns:1fr 1fr;gap:5px}"
                 ".g label{margin:0}.g input{width:auto}"
                 "</style></head><body><div class=c><h1>Photon</h1>"
                 "<div class=tabs>"
                 "<button class=\"tab%s\" onclick=s(0)>Device</button>"
                 "<button class=\"tab%s\" onclick=s(1)>Tickers</button>"
                 "<button class=\"tab%s\" onclick=s(2)>Portfolio</button>"
                 "</div><form method=POST action=/save-config>",
                 strcmp(active_tab, "device") == 0 ? " active" : "",
                 strcmp(active_tab, "tickers") == 0 ? " active" : "",
                 strcmp(active_tab, "portfolio") == 0 ? " active" : "");

    // Device tab
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<div class=\"t%s\" id=t0>"
                    "<label>Bright: <span id=v>%d%%</span></label>"
                    "<input type=range name=brightness min=1 max=20 value=%d "
                    "oninput=v.textContent=this.value+'%%'>",
                    strcmp(active_tab, "device") == 0 ? " active" : "", config->brightness,
                    config->brightness);

    // Location select
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Location</label><select name=location>");
    for (int i = 0; i < LOC_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<option value=%d%s>%s</option>", i,
                        config->location == i ? " selected" : "", location_get_name(i));
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</select>");

    // Currency select
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Currency</label><select name=currency>");
    for (int i = 0; i < CCY_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<option value=%d%s>%s</option>", i,
                        config->currency == i ? " selected" : "", currency_get_name(i));
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</select></div>");

    // Tickers tab
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<div class=\"t%s\" id=t1><label>Crypto</label><div class=g>",
                    strcmp(active_tab, "tickers") == 0 ? " active" : "");
    for (int i = 0; i < CRYPTO_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<label><input type=checkbox name=crypto_%d value=1%s>%s</label>", i,
                        (config->crypto & (1 << i)) ? " checked" : "", CRYPTO_ARR[i].to_str);
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "</div><label>Stocks</label><div class=g>");
    for (int i = 0; i < STK_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<label><input type=checkbox name=stk_%d value=1%s>%s</label>", i,
                        (config->stocks & (1 << i)) ? " checked" : "", STK_ARR[i].to_str);
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</div></div>");

    // Portfolio tab
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<div class=\"t%s\" id=t2><label>Portfolio</label>"
                    "<p style=color:#888;font-size:12px>T212 CSV</p>",
                    strcmp(active_tab, "portfolio") == 0 ? " active" : "");

    if (config->portfolio_count > 0)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<p style=color:#10b981>%d ticker%s</p>", config->portfolio_count,
                        config->portfolio_count == 1 ? "" : "s");
    }

    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "</div><button type=submit id=sv style=display:%s>Save & Reboot</button></form>"
                    "<form method=POST action=/upload enctype=multipart/form-data "
                    "style=display:%s id=pf onsubmit=\"return a()\">"
                    "<label>CSV File</label>"
                    "<input type=file name=portfolio accept=.csv id=f "
                    "onchange=\"var "
                    "b=u;b.disabled=!(this.files[0]&&this.files[0].name.endsWith('.csv'));b."
                    "textContent=b.disabled?'Select CSV':'Save All & Upload'\">"
                    "<label>P&L Mode</label><select name=portfolio_mode id=pm>"
                    "<option value=0%s>All-Time</option>"
                    "<option value=1%s>Daily</option></select>"
                    "<input type=hidden name=brightness id=B value=%d>"
                    "<input type=hidden name=location id=L value=%d>"
                    "<input type=hidden name=currency id=C value=%d>"
                    "<div id=K></div>"
                    "<button type=submit id=u disabled>Select CSV</button></form>",
                    strcmp(active_tab, "portfolio") == 0 ? "none" : "block",
                    strcmp(active_tab, "portfolio") == 0 ? "block" : "none",
                    config->portfolio_mode == 0 ? " selected" : "",
                    config->portfolio_mode == 1 ? " selected" : "", config->brightness,
                    config->location, config->currency);

    pos += snprintf(
        config_html_buf + pos, sizeof(config_html_buf) - pos,
        "</div><script>"
        "function a(){"
        "B.value=document.querySelector('[name=brightness]').value;"
        "L.value=document.querySelector('[name=location]').value;"
        "C.value=document.querySelector('[name=currency]').value;"
        "K.innerHTML='';"
        "document.querySelectorAll('[name^=crypto_]:checked').forEach(e=>{"
        "var "
        "h=document.createElement('input');h.type='hidden';h.name=e.name;h.value='1';K.appendChild("
        "h)});"
        "document.querySelectorAll('[name^=stk_]:checked').forEach(e=>{"
        "var "
        "h=document.createElement('input');h.type='hidden';h.name=e.name;h.value='1';K.appendChild("
        "h)});"
        "return true}"
        "function s(i){"
        "document.querySelectorAll('.tab').forEach((t,j)=>t.className=j==i?'tab active':'tab');"
        "document.querySelectorAll('.t').forEach((t,j)=>t.className=j==i?'t active':'t');"
        "sv.style.display=i==2?'none':'block';"
        "pf.style.display=i==2?'block':'none'}"
        "</script></body></html>");

    debug(DEBUG, "Generated HTML page: %d bytes (buffer size: %d)", pos,
          (int)sizeof(config_html_buf));

    if ((size_t)pos >= sizeof(config_html_buf))
    {
        debug(ERROR, "HTML buffer overflow! Generated %d bytes, buffer is %d", pos,
              (int)sizeof(config_html_buf));
    }

    return config_html_buf;
}
