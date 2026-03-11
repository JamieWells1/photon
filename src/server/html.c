/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

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
static char ticker_page_buf[512];
static char update_html_buf[8192];

const char* html_get_wifi_form(void) { return html_wifi_form; }

const char* html_get_success_page(void) { return html_success; }

const char* html_get_portfolio_success(void) { return html_portfolio_success; }

const char* html_get_portfolio_error(void) { return html_portfolio_error; }

const char* html_build_ticker_checking_page(const char* ticker)
{
    snprintf(ticker_page_buf, sizeof(ticker_page_buf),
             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
             "<!DOCTYPE html><html><head>"
             "<meta name=viewport content=width=device-width>"
             "<meta http-equiv=refresh content=5;url=/ticker-result>"
             "<title>Checking</title>"
             "<style>body{font:16px sans-serif;background:#4A90E2;color:#fff;"
             "display:flex;justify-content:center;align-items:center;height:100vh;margin:0}"
             "</style></head><body>Checking %s...</body></html>",
             ticker);
    return ticker_page_buf;
}

const char* html_build_ticker_error_page(const char* ticker)
{
    snprintf(ticker_page_buf, sizeof(ticker_page_buf),
             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
             "<!DOCTYPE html><html><head>"
             "<meta name=viewport content=width=device-width>"
             "<title>Error</title>"
             "<style>body{font:16px sans-serif;background:#4A90E2;color:#fff;"
             "display:flex;flex-direction:column;justify-content:center;align-items:center;"
             "height:100vh;margin:0}a{color:#fff;margin-top:20px}</style>"
             "</head><body>ticker not recognised : %s"
             "<a href=/watchlist>Back</a></body></html>",
             ticker);
    return ticker_page_buf;
}

static const char* html_ticker_still_checking =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
    "<!DOCTYPE html><html><head>"
    "<meta name=viewport content=width=device-width>"
    "<meta http-equiv=refresh content=2;url=/ticker-result>"
    "<title>Checking</title>"
    "<style>body{font:16px sans-serif;background:#4A90E2;color:#fff;"
    "display:flex;justify-content:center;align-items:center;height:100vh;margin:0}"
    "</style></head><body>Still checking...</body></html>";

const char* html_get_ticker_still_checking(void) { return html_ticker_still_checking; }

const char* html_build_config_page(const device_config_t* config, const char* active_tab)
{
    int pos = 0;

    // HTTP headers + minimal HTML/CSS start
    pos += snprintf(
        config_html_buf + pos, sizeof(config_html_buf) - pos,
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><meta name=viewport content=width=device-width>"
        "<title>Photon</title><style>"
        "*{box-sizing:border-box;margin:0}body{background:#4A90E2;padding:10px;font:14px "
        "sans-serif}"
        ".c{background:#fff;padding:12px;max-width:400px;margin:0 auto}"
        "h1{margin:0 0 8px;font-size:20px}"
        ".tabs{display:flex;border-bottom:1px solid #ddd}"
        ".tab{flex:1;padding:6px;text-align:center;cursor:pointer;border:none;background:none;"
        "color:#666}"
        ".tab.a{color:#4A90E2;border-bottom:2px solid #4A90E2}"
        ".t{display:none}.t.a{display:block}"
        "label{display:block;margin:8px 0 2px}.sh{margin:14px 0 10px}"
        "input,select{width:100%%;padding:5px;border:1px solid #ddd}"
        "button{width:100%%;background:#4A90E2;color:#fff;border:0;padding:8px;margin-top:8px;"
        "cursor:pointer}"
        ".g{display:grid;grid-template-columns:1fr 1fr;gap:4px}"
        ".g label{margin:0}.g input{width:auto}"
        ".tl{list-style:none;padding:0}.tl "
        "li{display:flex;justify-content:space-between;padding:3px 0}"
        ".rm{color:red;text-decoration:none;cursor:pointer}"
        ".row{display:flex;gap:4px}.row input{flex:1}.row button{width:auto;margin:0;padding:5px "
        "10px}"
        "</style><body><div class=c><h1>Photon</h1>"
        "<div class=tabs>"
        "<button class=\"tab%s\" onclick=s(0)>Device</button>"
        "<button class=\"tab%s\" onclick=s(1)>Watchlist</button>"
        "<button class=\"tab%s\" onclick=s(2)>Portfolio</button>"
        "</div><form method=POST action=/save-config>",
        strcmp(active_tab, "device") == 0 ? " a" : "",
        strcmp(active_tab, "watchlist") == 0 || strcmp(active_tab, "tickers") == 0 ? " a" : "",
        strcmp(active_tab, "portfolio") == 0 ? " a" : "");

    // Device tab
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<div class=\"t%s\" id=t0>"
                    "<label>Bright: <span id=v>%d%%</span></label>"
                    "<input type=range name=brightness min=1 max=20 value=%d "
                    "oninput=v.textContent=this.value+'%%'>",
                    strcmp(active_tab, "device") == 0 ? " a" : "", config->brightness,
                    config->brightness);

    // Location select
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Location</label><select name=location>");
    for (int i = 0; i < LOC_COUNT; i++)
    {
        const char* sel = config->location == i ? " selected" : "";
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<option value=%d%s>%s</option>", i, sel, location_get_name(i));
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</select>");

    // Currency select
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>Currency</label><select name=currency>");
    for (int i = 0; i < CCY_COUNT; i++)
    {
        const char* sel = config->currency == i ? " selected" : "";
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<option value=%d%s>%s</option>", i, sel, currency_get_name(i));
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</select></div>");

    // Watchlist tab
    bool watchlist_active =
        (strcmp(active_tab, "watchlist") == 0 || strcmp(active_tab, "tickers") == 0);
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<div class=\"t%s\" id=t1>", watchlist_active ? " a" : "");

    // Stocks section
    if (config->portfolio_count > 0)
    {
        // Portfolio user: show all tickers as checkboxes, max 10 selected
        int checked_count = 0;
        for (int i = 0; i < config->portfolio_count && i < MAX_STOCK_TICKERS; i++)
        {
            if (config->stocks & (1ULL << i)) checked_count++;
        }

        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<label class=sh>Stocks <span id=sc>(%d/20)</span></label>"
                        "<div class=g id=sg onchange=ck()>",
                        checked_count);
        for (int i = 0; i < config->portfolio_count && i < MAX_STOCK_TICKERS; i++)
        {
            bool checked = (config->stocks & (1ULL << i)) != 0;
            // Combined limit of 20 (portfolio + manual tickers)
            int total_count = checked_count + config->stock_ticker_count;
            bool disabled = !checked && total_count >= 20;
            pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                            "<label><input type=checkbox name=s%d%s%s>%s</label>", i,
                            checked ? " checked" : "", disabled ? " disabled" : "",
                            config->portfolio_tickers[i]);
        }
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</div>");
    }
    else
    {
        // No portfolio: show message + upload button
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<p style=color:#888;margin:8px 0>No portfolio uploaded</p>"
                        "<button type=button onclick=s(2)>Upload Portfolio</button>");
    }

    // Always show manual ticker input
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label>%s</label>"
                    "<div class=row>"
                    "<input type=text id=ti placeholder=\"e.g. AAPL\" maxlength=9>"
                    "<button type=button onclick=add()>Add</button>"
                    "</div>"
                    "<ul class=tl id=tlist></ul>"
                    "<p id=maxmsg style=font-size:11px;color:#888;display:none>Max 20 total</p>",
                    config->portfolio_count > 0 ? "Add More Tickers" : "Or add manually");

    // Crypto section (unchanged)
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<label class=sh>Crypto</label><div class=g>");
    for (int i = 0; i < CRYPTO_COUNT; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<label><input type=checkbox name=c%d%s>%s</label>", i,
                        (config->crypto & (1 << i)) ? " checked" : "", CRYPTO_ARR[i].to_str);
    }
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "</div></div>");

    // Portfolio tab
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "<div class=\"t%s\" id=t2><label>Portfolio</label>"
                    "<p style=color:#888;font-size:12px>CSV</p>",
                    strcmp(active_tab, "portfolio") == 0 ? " a" : "");

    if (config->portfolio_count > 0)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                        "<p style=color:#10b981>%d ticker%s</p>", config->portfolio_count,
                        config->portfolio_count == 1 ? "" : "s");
    }

    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "</div><button type=button onclick=saveCfg() id=sv style=display:%s>Save & "
                    "Reboot</button></form>"
                    "<form method=POST action=/upload enctype=multipart/form-data "
                    "style=display:%s id=pf onsubmit=\"return a()\">"
                    "<label>CSV File</label>"
                    "<input type=file name=portfolio accept=.csv id=f "
                    "onchange=\"u.disabled=!(this.files[0]&&this.files[0].name.endsWith('.csv'));"
                    "u.textContent=u.disabled?'Select CSV':'Save All & Upload'\">"
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

    // JavaScript with manual ticker management
    pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos,
                    "</div><script>"
                    "function q(s){return document.querySelectorAll(s)}"
                    "function g(i){return document.getElementById(i)}"
                    "var mt=[");  // manualTickers array

    // Initialize with existing manual tickers from config
    for (int i = 0; i < config->stock_ticker_count && i < 10; i++)
    {
        pos += snprintf(config_html_buf + pos, sizeof(config_html_buf) - pos, "%s'%s'",
                        i > 0 ? "," : "", config->stock_tickers[i]);
    }

    pos += snprintf(
        config_html_buf + pos, sizeof(config_html_buf) - pos,
        "],D=document,E='createElement',H='hidden',I='innerHTML',N='none',B='block';"
        "function r(){var l=g('tlist'),m=g('maxmsg');if(!l)return;l[I]='';"
        "mt.forEach((t,i)=>l[I]+='<li><span>'+t+'</span><a class=rm onclick=x('+i+')>X</a>');"
        "var pc=q('#sg "
        "[type=checkbox]:checked').length||0;if(m)m.style.display=(pc+mt.length)>=20?B:N;ck()}"
        "function add(){var i=g('ti');if(!i)return;var t=i.value.trim().toUpperCase();"
        "var pc=q('#sg "
        "[type=checkbox]:checked').length||0;if(!t||t.length>9||(pc+mt.length)>=20||mt.indexOf(t)>="
        "0)return;mt.push(t);i.value='';r()}"
        "function x(i){mt.splice(i,1);r()}"
        "function ck(){var n=q('#sg [type=checkbox]:checked').length||0,c=q('#sg "
        "[type=checkbox]'),total=n+mt.length;"
        "c.forEach(e=>{if(!e.checked)e.disabled=total>=20});var "
        "s=g('sc');if(s)s.textContent='('+total+'/20)'}"
        "function "
        "a(){B.value=q('[name=brightness]')[0].value;L.value=q('[name=location]')[0].value;"
        "C.value=q('[name=currency]')[0].value;K[I]='';"
        "q('[name^=c]:checked,[name^=s]:checked').forEach(e=>{"
        "var h=D[E]('input');h.type=H;h.name=e.name;h.value='1';K.appendChild(h)});return 1}"
        "function saveCfg(){var f=D.forms[0];mt.forEach((t,i)=>{"
        "var h=D[E]('input');h.type=H;h.name='mt_'+i;h.value=t;f.appendChild(h)});f.submit()}"
        "function s(i){q('.tab').forEach((t,j)=>t.className=j==i?'tab a':'tab');"
        "q('.t').forEach((t,j)=>t.className=j==i?'t a':'t');"
        "sv.style.display=i==2?N:B;pf.style.display=i==2?B:N}setTimeout(r,0);"
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

const char* html_build_update_page(bool update_available, const char* remote_version)
{
    int pos = 0;

    pos += snprintf(
        update_html_buf + pos, sizeof(update_html_buf) - pos,
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><meta name=viewport content=width=device-width>"
        "<title>Photon Update</title><style>"
        "*{box-sizing:border-box;margin:0}body{background:#4A90E2;padding:20px;font:14px "
        "sans-serif;color:#333}"
        ".c{background:#fff;padding:20px;max-width:500px;margin:0 auto;border-radius:8px}"
        "h1{margin:0 0 16px;font-size:24px}"
        ".warn{background:#FFF3CD;border:1px solid "
        "#FFC107;padding:12px;border-radius:4px;margin:16px "
        "0}"
        ".info{background:#D1ECF1;border:1px solid "
        "#17A2B8;padding:12px;border-radius:4px;margin:16px "
        "0}"
        "button{width:100%%;background:#4A90E2;color:#fff;border:0;padding:12px;margin:8px "
        "0;cursor:pointer;border-radius:4px;font-size:16px}"
        "button:disabled{background:#ccc;cursor:not-allowed}"
        ".progress{display:none;margin:16px 0;text-align:center}"
        ".log{background:#f5f5f5;border:1px solid #ddd;padding:8px;margin:16px "
        "0;font-family:monospace;font-size:11px;max-height:200px;overflow-y:auto}"
        "</style></head><body><div class=c><h1>Photon Firmware Update</h1>");

    if (update_available && remote_version)
    {
        pos += snprintf(update_html_buf + pos, sizeof(update_html_buf) - pos,
                        "<div class=info>Update available: Version %s</div>", remote_version);
    }
    else
    {
        pos += snprintf(update_html_buf + pos, sizeof(update_html_buf) - pos,
                        "<div class=info>Your firmware is up to date</div>");
    }

    pos += snprintf(
        update_html_buf + pos, sizeof(update_html_buf) - pos,
        "<div class=warn><strong>Note:</strong> WebUSB automatic flashing works in Chrome/Edge. "
        "For other browsers, the firmware will download and you can manually flash via USB.</div>"
        "<div id=status></div>"
        "<div class=progress id=prog style=display:none><div>Updating...</div></div>"
        "<div class=log id=log style=display:none></div>"
        "<button onclick=startUpdate() id=btn%s>Download & Install Update</button>"
        "<div "
        "style='margin-top:16px;padding:12px;background:#f5f5f5;border-radius:4px;display:none' "
        "id=manual>"
        "<strong>Manual Installation:</strong><ol style='margin:8px 0 0 20px;padding:0'>"
        "<li>Download has started (check your downloads folder)</li>"
        "<li>Connect device to computer via USB</li>"
        "<li>Device appears as 'RPI-RP2' drive</li>"
        "<li>Drag the .uf2 file onto the drive</li>"
        "<li>Device reboots automatically with new firmware</li></ol></div>"
        "</div><script>"
        "const s=document.getElementById('status'),"
        "p=document.getElementById('prog'),"
        "b=document.getElementById('btn'),"
        "l=document.getElementById('log'),"
        "m=document.getElementById('manual');"
        "function log(msg){const d=new Date();l.style.display='block';"
        "l.innerHTML+=d.toLocaleTimeString()+' - '+msg+'<br>';l.scrollTop=l.scrollHeight}"
        "function show(msg,type){s.innerHTML='<div class='+(type||'info')+'>'+msg+'</div>'}"
        "function downloadFirmware(){"
        "const a=document.createElement('a');"
        "a.href='http://%s:8090/firmware/latest.uf2';"
        "a.download='photon-firmware.uf2';"
        "document.body.appendChild(a);a.click();document.body.removeChild(a);"
        "log('Firmware download started')}"
        "async function webUSBFlash(uf2Data){"
        "log('Requesting USB device access...');"
        "const dev=await "
        "navigator.usb.requestDevice({filters:[{vendorId:0x2E8A,productId:0x0003}]});"
        "if(!dev){throw new Error('No device selected')}"
        "log('Opening device connection...');"
        "await dev.open();"
        "if(dev.configuration===null){await dev.selectConfiguration(1)}"
        "log('Claiming USB interface...');"
        "await dev.claimInterface(1);"
        "log('Starting firmware transfer...');"
        "const BLOCK_SIZE=512;const blocks=Math.ceil(uf2Data.byteLength/BLOCK_SIZE);"
        "for(let i=0;i<blocks;i++){"
        "const offset=i*BLOCK_SIZE;"
        "const chunk=uf2Data.slice(offset,Math.min(offset+BLOCK_SIZE,uf2Data.byteLength));"
        "const padded=new Uint8Array(BLOCK_SIZE);padded.set(new Uint8Array(chunk));"
        "await dev.transferOut(2,padded);"
        "if(i%%10===0){const pct=Math.round((i/blocks)*100);log('Writing: '+pct+'%%')}}"
        "log('Transfer complete, closing connection...');"
        "await dev.close();"
        "log('Firmware flashed successfully!')}"
        "async function startUpdate(){"
        "const hasWebUSB=!!navigator.usb;"
        "b.disabled=true;"
        "try{"
        "if(hasWebUSB){p.style.display='block';log('Starting WebUSB update...')}"
        "log('Downloading firmware from server...');"
        "const resp=await fetch('http://%s:8090/firmware/latest.uf2');"
        "if(!resp.ok)throw new Error('Download failed: '+resp.statusText);"
        "const uf2=await resp.arrayBuffer();"
        "log('Downloaded '+Math.round(uf2.byteLength/1024)+'KB');"
        "if(hasWebUSB){"
        "log('Triggering device bootloader mode...');"
        "await fetch('/update',{method:'POST'});"
        "log('Waiting for device to enter bootloader (3 seconds)...');"
        "await new Promise(r=>setTimeout(r,3000));"
        "try{await webUSBFlash(uf2);"
        "show('✓ Update successful! Device is rebooting...','info');p.style.display='none';"
        "}catch(usbErr){"
        "log('WebUSB failed: '+usbErr.message);"
        "log('Falling back to manual installation...');"
        "downloadFirmware();"
        "await fetch('/update',{method:'POST'});"
        "m.style.display='block';"
        "show('Download started. Follow manual installation steps below.','info');"
        "p.style.display='none'}}"
        "else{downloadFirmware();"
        "await new Promise(r=>setTimeout(r,500));"
        "await fetch('/update',{method:'POST'});"
        "m.style.display='block';"
        "show('Download started. Connect device via USB and follow steps below.','info')}"
        "}catch(e){log('Error: '+e.message);"
        "show('Update failed: '+e.message,'warn');"
        "b.disabled=false;p.style.display='none'}}"
        "</script></body></html>",
        update_available ? "" : " disabled", PROXY_HOST, PROXY_HOST);

    return update_html_buf;
}
