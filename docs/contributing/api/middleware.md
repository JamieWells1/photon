# Middleware API

The Middleware API provides simple, synchronous WiFi connectivity and HTTP request handling with built-in visual feedback.

## WiFi Management

### wifi_connect

Connect to a WiFi network with visual feedback.

```c
int wifi_connect(const char* ssid, const char* password, Matrix* mtrx);
```

**Parameters:**

- `ssid` - Network SSID
- `password` - Network password
- `mtrx` - Matrix display for status feedback

**Returns:**

- `0` on success
- `-1` on failure (timeout or connection error)

**Behavior:**

- Blocks until connected or timeout
- Displays "WIFI..." with animated dots during connection
- Shows "TIMEOUT" error message on failure
- Timeout configured by `MAX_WIFI_CONNECTION_ATTEMPTS` in `const.h` (default: 100 attempts × 100ms = 10 seconds)

**Example:**

```c
if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, &matrix) == 0) {
    // Connected successfully
    // Make HTTP requests, etc.
    wifi_disconnect();
}
```

---

### wifi_disconnect

Disconnect from WiFi network to save power.

```c
void wifi_disconnect(void);
```

**Example:**

```c
wifi_disconnect();
debug(INFO, "WiFi disconnected to save power");
```

---

### wifi_init_ap

Initialize WiFi in Access Point mode for configuration.

```c
int wifi_init_ap(const char* ssid, const char* password, Matrix* mtrx);
```

**Parameters:**

- `ssid` - Access point SSID
- `password` - Access point password (can be `NULL` for open network)
- `mtrx` - Matrix display for status feedback

**Returns:**

- `0` on success
- `-1` on failure

**Behavior:**

- Creates a WiFi access point at `192.168.4.1`
- Starts a DHCP server for client connections
- Displays "START..." during initialization
- Used for captive portal WiFi configuration

**Example:**

```c
wifi_init_ap("Photon-Setup", NULL, &matrix);
// AP is now active, start HTTP and DNS servers
http_server_start();
dns_server_start();
```

---

### wifi_is_ap_mode

Check if WiFi is in Access Point mode.

```c
bool wifi_is_ap_mode(void);
```

**Returns:**

- `true` if in AP mode
- `false` if in client mode or disconnected

**Example:**

```c
if (wifi_is_ap_mode()) {
    debug(INFO, "Running in AP mode");
} else {
    debug(INFO, "Running in client mode");
}
```

---

## Flash Configuration

### flash_config_read

Read WiFi credentials from flash memory.

```c
bool flash_config_read(wifi_config_t* config);
```

**Parameters:**

- `config` - Pointer to `wifi_config_t` structure to populate

**Returns:**

- `true` if valid config was read
- `false` if no valid config exists

**Example:**

```c
wifi_config_t config;
if (flash_config_read(&config)) {
    wifi_connect(config.ssid, config.password, &matrix);
}
```

---

### flash_config_write

Write WiFi credentials to flash memory.

```c
bool flash_config_write(const wifi_config_t* config);
```

**Parameters:**

- `config` - Pointer to `wifi_config_t` structure to save

**Returns:**

- `true` if write succeeded
- `false` if write failed

**Example:**

```c
wifi_config_t config;
strncpy(config.ssid, "MyNetwork", sizeof(config.ssid));
strncpy(config.password, "MyPassword", sizeof(config.password));
config.magic = FLASH_CONFIG_MAGIC;
flash_config_write(&config);
```

---

### flash_config_exists

Check if valid WiFi configuration exists in flash.

```c
bool flash_config_exists(void);
```

**Returns:**

- `true` if valid config exists
- `false` otherwise

**Example:**

```c
if (!flash_config_exists()) {
    // Enter setup mode
    wifi_init_ap("Photon-Setup", NULL, &matrix);
}
```

---

### flash_config_erase

Erase WiFi configuration from flash (factory reset).

```c
void flash_config_erase(void);
```

**Behavior:**

- Erases stored WiFi credentials
- Device will enter setup mode on next boot

**Example:**

```c
// Factory reset
flash_config_erase();
watchdog_enable(1, 1);  // Reboot
while (1);
```

---

## HTTP Server (Captive Portal)

### http_server_start

Start HTTP server for WiFi configuration.

```c
void http_server_start(void);
```

**Behavior:**

- Starts HTTP server on port 80
- Serves WiFi configuration web page
- Handles credential submission

**Example:**

```c
wifi_init_ap("Photon-Setup", NULL, &matrix);
http_server_start();
```

---

### http_server_stop

Stop HTTP server.

```c
void http_server_stop(void);
```

---

### http_server_credentials_saved

Check if user has submitted WiFi credentials.

```c
bool http_server_credentials_saved(void);
```

**Returns:**

- `true` if credentials have been saved
- `false` otherwise

**Example:**

```c
while (!http_server_credentials_saved()) {
    cyw43_arch_poll();
    sleep_ms(10);
}
// Credentials saved, reboot
```

---

## DNS Server (Captive Portal)

### dns_server_start

Start DNS server for captive portal.

```c
void dns_server_start(void);
```

**Behavior:**

- Responds to all DNS queries with AP IP (192.168.4.1)
- Enables captive portal redirect

**Example:**

```c
wifi_init_ap("Photon-Setup", NULL, &matrix);
dns_server_start();
http_server_start();
```

---

### dns_server_stop

Stop DNS server.

```c
void dns_server_stop(void);
```

---

## HTTP Requests

### http_get

Send an HTTP GET request with visual feedback.

```c
typedef void (*http_response_callback_t)(const char* body, size_t len, bool complete);

int http_get(const char* host_url,
             const char* url_sub_path,
             http_response_callback_t callback,
             Matrix* mtrx);
```

**Parameters:**

- `host_url` - Hostname (e.g., `"api.example.com"`)
- `url_sub_path` - Path (e.g., `"/v1/weather?lat=51.5&lon=0.1"`)
- `callback` - Function called with response data
- `mtrx` - Matrix display for status feedback

**Returns:**

- `0` on success
- `-1` on failure (timeout, DNS error, connection error)

**Callback Parameters:**

- `body` - Response body content
- `len` - Length of body
- `complete` - `true` if this is the final callback with complete data

**Behavior:**

- Blocks until response received or timeout (30 seconds)
- Displays "DATA..." with animated dots while waiting
- Shows "TIMEOUT" error message on failure
- Callback may be invoked multiple times as data streams in
- Final callback has `complete = true`

**Example:**

```c
void handle_weather_response(const char* body, size_t len, bool complete) {
    if (!complete) return;  // Wait for complete response

    // Parse JSON response
    float temp = parse_temperature(body);
    int code = parse_weather_code(body);

    debug(DEBUG, "Temperature: %.1f°C, Code: %d", temp, code);
}

int result = http_get(
    HOST_URL_WEATHER,
    "/v1/forecast?latitude=51.5&longitude=-0.1",
    handle_weather_response,
    &matrix
);

if (result != 0) {
    debug(INFO, "HTTP request failed");
}
```

---

**Next:** [Building a Mode Tutorial](../building-a-mode.md) →
