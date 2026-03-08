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

static const char* html_portfolio_success =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
    "<meta charset=\"UTF-8\"><title>Portfolio Uploaded</title>"
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
    "a{display:inline-block;margin-top:20px;color:#4A90E2;text-decoration:none;font-weight:600}"
    "</style></head><body>"
    "<div class=\"container\"><div class=\"checkmark\">✓</div><h1>Portfolio Uploaded!</h1>"
    "<p>Successfully loaded portfolio tickers</p>"
    "<a href=\"/config\">← Back to Settings</a></div></body></html>";

static const char* html_portfolio_error =
    "HTTP/1.1 400 Bad Request\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n\r\n"
    "Failed to parse CSV";

static char config_html_buf[8192];

const char* html_get_wifi_form(void) { return html_wifi_form; }

const char* html_get_success_page(void) { return html_success; }

const char* html_get_portfolio_success(void) { return html_portfolio_success; }

const char* html_get_portfolio_error(void) { return html_portfolio_error; }

const char* html_build_config_page(const device_config_t* config)
{
    int pos = 0;
    pos += snprintf(
        config_html_buf + pos, sizeof(config_html_buf) - pos,
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width\">"
        "<meta charset=\"UTF-8\"><title>Photon Config</title>"
        "<style>"
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe "
        "UI',Roboto,sans-serif;background:linear-gradient(135deg,#4A90E2 0%%,#357ABD "
        "100%%);margin:0;padding:20px;min-height:100vh;display:flex;align-items:center;justify-"
        "content:center}"
        ".container{background:white;border-radius:12px;box-shadow:0 8px 24px "
        "rgba(0,0,0,0.15);padding:32px;max-width:400px;width:100%%}"
        "h1{color:#333;margin:0 0 24px;font-size:28px;font-weight:600;text-align:center}"
        "label{display:block;color:#555;font-size:14px;font-weight:500;margin-bottom:8px;margin-"
        "top:16px}"
        "input,select{width:100%%;padding:12px;border:2px solid "
        "#e0e0e0;border-radius:8px;font-size:16px;box-sizing:border-box;transition:border-color "
        "0.3s}"
        "input:focus,select:focus{outline:none;border-color:#4A90E2}"
        "button{width:100%%;background:linear-gradient(135deg,#4A90E2 0%%,#357ABD "
        "100%%);color:white;border:none;padding:14px;border-radius:8px;font-size:16px;font-weight:"
        "600;cursor:pointer;transition:transform 0.2s,box-shadow 0.2s;margin-top:20px}"
        "button:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(74,144,226,0.4)}"
        "button:active{transform:translateY(0)}"
        ".subtitle{color:#888;font-size:14px;text-align:center;margin-bottom:24px}"
        ".range-value{display:inline-block;color:#4A90E2;font-weight:600;margin-left:8px}"
        ".warning{display:none;color:#dc2626;font-size:13px;margin-top:4px;font-weight:500}"
        ".warning.show{display:block}"
        ".ticker-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:8px}"
        ".ticker-grid label{display:flex;align-items:center;gap:8px;margin:0;cursor:pointer}"
        ".ticker-grid input[type=checkbox]{width:auto;margin:0;padding:0}"
        "</style></head><body>"
        "<div class=\"container\"><h1>Device Settings</h1>"
        "<p class=\"subtitle\">Configure your Photon display</p>"
        "<form method=\"POST\" action=\"/save-config\" "
        "enctype=\"application/x-www-form-urlencoded\">"
        "<label>Brightness <span class=\"range-value\" id=\"brightnessValue\">%d%%</span></label>"
        "<input type=\"range\" name=\"brightness\" id=\"brightness\" min=\"1\" max=\"20\" "
        "value=\"%d\" step=\"1\" oninput=\"updateBrightness(this.value)\">"
        "<p class=\"warning\" id=\"powerWarning\"%s>⚠️ External power supply highly recommended to "
        "avoid overload</p>",
        config->brightness, config->brightness,
        config->brightness >= 10 ? " class=\"warning show\"" : "");

    // Location select
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Location</label><select name=\"location\">");
    for (int i = 0; i < LOC_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<option value=\"%d\"%s>%s</option>", i,
                        config->location == i ? " selected" : "", location_get_name(i));
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</select>");

    // Currency select
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Currency</label><select name=\"currency\">");
    for (int i = 0; i < CCY_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<option value=\"%d\"%s>%s</option>", i,
                        config->currency == i ? " selected" : "", currency_get_name(i));
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</select>");

    // Crypto checkboxes
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Crypto</label><div class=\"ticker-grid\">");
    for (int i = 0; i < CRYPTO_COUNT; i++)
    {
        pos +=
            snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                     "<label><input type=\"checkbox\" name=\"crypto_%d\" value=\"1\"%s>%s</label>",
                     i, (config->crypto & (1 << i)) ? " checked" : "", CRYPTO_ARR[i].to_str);
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</div>");

    // Stock checkboxes
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Stocks</label><div class=\"ticker-grid\">");
    for (int i = 0; i < STK_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<label><input type=\"checkbox\" name=\"stk_%d\" value=\"1\"%s>%s</label>",
                        i, (config->stocks & (1 << i)) ? " checked" : "", STK_ARR[i].to_str);
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</div>");

    // Portfolio upload section
    pos += snprintf(
        config_html_buf + pos, sizeof(config_html_buf) - pos,
        "<label>Portfolio Upload</label>"
        "<p style=\"color:#888;font-size:12px;margin:4px 0 8px;\">Upload T212 CSV export to "
        "track your portfolio (max 30 tickers)</p>");

    if (config->portfolio_count > 0)
    {
        pos += snprintf(
            config_html_buf + pos, sizeof(config_html_buf) - pos,
            "<p style=\"color:#10b981;font-size:13px;font-weight:500;margin-bottom:8px;\">✓ "
            "%d ticker%s loaded</p>",
            config->portfolio_count, config->portfolio_count == 1 ? "" : "s");
    }

    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "</form>"
                    "<form method=\"POST\" action=\"/upload-portfolio\" "
                    "enctype=\"multipart/form-data\" style=\"margin-top:8px;\">"
                    "<input type=\"file\" name=\"portfolio\" accept=\".csv\" "
                    "style=\"font-size:14px;padding:8px;\">"
                    "<button type=\"submit\" style=\"margin-top:12px;\">Upload Portfolio</button>"
                    "</form>"
                    "<form method=\"POST\" action=\"/save-config\" "
                    "enctype=\"application/x-www-form-urlencoded\" style=\"margin-top:8px;\">"
                    "<button type=\"submit\">Save & Reboot</button>");

    // Submit and script
    pos +=
        snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                 "</form></div>"
                 "<script>"
                 "function updateBrightness(val){"
                 "document.getElementById('brightnessValue').textContent=val+'%%';"
                 "var warning=document.getElementById('powerWarning');"
                 "if(val>10){warning.classList.add('show');}else{warning.classList.remove('show');}"
                 "}"
                 "</script>"
                 "</body></html>");

    return config_html_buf;
}
