#include <wifi.h>

#include <macro.h>

#include <stdio.h>

#include <pico/cyw43_arch.h>

static bool initialized = false;
static bool connected = false;
static WifiState current_state = WIFI_DISCONNECTED;

static int wifi_init(void)
{
    if (initialized)
    {
        return 0;
    }

    if (cyw43_arch_init())
    {
        debug("Wi-Fi init failed");
        return -1;
    }

    debug("Wi-Fi initialized");
    initialized = true;
    return 0;
}

static void wifi_enable_sta_mode(void)
{
    cyw43_arch_enable_sta_mode();
    debug("Wi-Fi station mode enabled");
}

static int wifi_connect(const char* ssid, const char* password, uint32_t timeout_ms)
{
    debug("Connecting to Wi-Fi...");

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms))
    {
        debug("Failed to connect to Wi-Fi");
        connected = false;
        return 1;
    }

    debug("Connected to Wi-Fi");
    connected = true;
    return 0;
}

int wifi_join(const char* ssid, const char* password)
{
    if (connected)
    {
        current_state = WIFI_CONNECTED;
        return 0;
    }

    if (!initialized)
    {
        if (wifi_init() != 0)
        {
            current_state = WIFI_FAILED;
            return -1;
        }
        wifi_enable_sta_mode();
    }

    current_state = WIFI_CONNECTING;
    int result = wifi_connect(ssid, password, 30000);
    current_state = (result == 0) ? WIFI_CONNECTED : WIFI_FAILED;
    return result;
}

int wifi_join_async(const char* ssid, const char* password)
{
    if (connected)
    {
        current_state = WIFI_CONNECTED;
        return 0;
    }

    if (current_state == WIFI_CONNECTING)
    {
        int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        debug("WiFi link status: %d", link_status);

        if (link_status == CYW43_LINK_UP)
        {
            debug("WiFi connected!");
            connected = true;
            current_state = WIFI_CONNECTED;
            return 0;
        }
        else if (link_status == CYW43_LINK_FAIL || link_status == CYW43_LINK_BADAUTH ||
                 link_status == CYW43_LINK_NONET)
        {
            debug("WiFi connection failed (status: %d)", link_status);
            connected = false;
            current_state = WIFI_FAILED;
            return -1;
        }

        return 1;
    }

    if (!initialized)
    {
        if (wifi_init() != 0)
        {
            current_state = WIFI_FAILED;
            return -1;
        }
        wifi_enable_sta_mode();
    }

    debug("Starting async WiFi connection...");
    current_state = WIFI_CONNECTING;

    cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    return 1;
}

WifiState wifi_get_state(void) { return current_state; }

void wifi_disconnect(void)
{
    if (!connected)
    {
        debug("WiFi already disconnected - resetting state");
        current_state = WIFI_DISCONNECTED;
        return;
    }

    debug("Disconnecting WiFi to save power...");
    cyw43_arch_deinit();
    connected = false;
    initialized = false;
    current_state = WIFI_DISCONNECTED;
    debug("WiFi disconnected");
}

// ==========================
// WiFi Connection Manager
// ==========================

void wifi_manager_init(WifiConnectionManager* mgr, int max_attempts)
{
    mgr->attempt_count = 0;
    mgr->max_attempts = max_attempts;
    mgr->failed = false;
    mgr->in_progress = false;
}

void wifi_manager_start(WifiConnectionManager* mgr, const char* ssid, const char* password)
{
    debug("WiFi Manager: Starting connection...");
    mgr->in_progress = true;
    mgr->failed = false;
    mgr->attempt_count = 0;
    wifi_join_async(ssid, password);
}

WifiState wifi_manager_update(WifiConnectionManager* mgr, const char* ssid, const char* password)
{
    if (!mgr->in_progress)
    {
        return wifi_get_state();
    }

    WifiState state = wifi_get_state();

    if (state == WIFI_CONNECTED)
    {
        debug("WiFi Manager: Connected successfully");
        mgr->in_progress = false;
        mgr->failed = false;
        return WIFI_CONNECTED;
    }

    if (state == WIFI_FAILED)
    {
        debug("WiFi Manager: Connection failed");
        mgr->in_progress = false;
        mgr->failed = true;
        wifi_disconnect();
        return WIFI_FAILED;
    }

    if (state == WIFI_CONNECTING)
    {
        mgr->attempt_count++;
        debug("WiFi Manager: Connecting... (attempt %d/%d)", mgr->attempt_count, mgr->max_attempts);

        if (mgr->attempt_count >= mgr->max_attempts)
        {
            debug("WiFi Manager: Connection timeout");
            mgr->in_progress = false;
            mgr->failed = true;
            wifi_disconnect();
            return WIFI_FAILED;
        }

        wifi_join_async(ssid, password);
        return WIFI_CONNECTING;
    }

    return state;
}

bool wifi_manager_is_connecting(const WifiConnectionManager* mgr) { return mgr->in_progress; }

bool wifi_manager_has_failed(const WifiConnectionManager* mgr) { return mgr->failed; }

int wifi_manager_get_attempts(const WifiConnectionManager* mgr) { return mgr->attempt_count; }

void wifi_manager_reset(WifiConnectionManager* mgr)
{
    debug("WiFi Manager: Resetting");
    mgr->attempt_count = 0;
    mgr->failed = false;
    mgr->in_progress = false;
}

bool wifi_manager_should_connect(const WifiConnectionManager* mgr)
{
    return !mgr->in_progress && !mgr->failed && wifi_get_state() == WIFI_DISCONNECTED;
}

void wifi_manager_connect_if_needed(WifiConnectionManager* mgr, const char* ssid,
                                    const char* password, int max_attempts)
{
    if (wifi_manager_should_connect(mgr))
    {
        debug("Starting WiFi connection...");
        wifi_manager_init(mgr, max_attempts);
        wifi_manager_start(mgr, ssid, password);
    }
}

void wifi_manager_disconnect_if_connected(void)
{
    if (wifi_get_state() == WIFI_CONNECTED)
    {
        debug("Auto-disconnecting WiFi to save power");
        wifi_disconnect();
    }
}

void wifi_append_connecting_dots(char* buffer, size_t buffer_size, int* dot_counter)
{
    size_t len = 0;
    for (size_t i = 0; buffer[i] != '\0' && i < buffer_size; i++)
    {
        len = i + 1;
    }

    while (len > 0 && buffer[len - 1] == '.')
    {
        buffer[--len] = '\0';
    }

    int num_dots = (*dot_counter % 3) + 1;
    for (int i = 0; i < num_dots && len + i + 1 < buffer_size; i++)
    {
        buffer[len + i] = '.';
        buffer[len + i + 1] = '\0';
    }

    (*dot_counter)++;
}
