# Middleware API

The Middleware API provides WiFi connectivity, HTTP requests, and persistent flash storage.

## Flash Memory

Persistent device configuration stored in the last sector of flash. Survives reboots and power cycles. Erased by factory reset.

### device_config_t

```c
typedef struct {
    uint32_t magic;        // Validation marker (set automatically)
    char ssid[64];         // WiFi network name
    char password[64];     // WiFi password
    uint8_t brightness;    // Display brightness (1-25%)
    uint8_t location;      // Location index for weather
    uint8_t currency;      // Currency index for tickers
    uint32_t crc32;        // Integrity check (set automatically)
} device_config_t;
```

---

### flash_memory_read

Read device configuration from flash.

```c
bool flash_memory_read(device_config_t* config);
```

**Parameters:**

- `config` - Pointer to config struct to populate

**Returns:**

- `true` if valid config was found in flash
- `false` if no config exists or data is corrupt (config is populated with defaults)

On failure, the config struct is zeroed and populated with default values (brightness=8%, location=London, currency=GBP). This means callers never need to handle the failure case for reading settings -- defaults are always safe to use.

**Example:**

```c
device_config_t config;
if (flash_memory_read(&config)) {
    // Valid config found, connect to WiFi
    wifi_connect(config.ssid, config.password, mtrx);
} else {
    // No config -- defaults applied, enter setup mode
}
```

---

### flash_memory_write

Write device configuration to flash.

```c
bool flash_memory_write(const device_config_t* config);
```

**Parameters:**

- `config` - Pointer to config struct to write

**Returns:**

- `true` on success
- `false` if config is NULL

Magic number and CRC32 are set automatically. Disables interrupts during the write operation.

**Example:**

```c
device_config_t config;
flash_memory_read(&config);  // Load existing config
config.brightness = 10;       // Modify a field
flash_memory_write(&config);  // Save back
```

---

### flash_memory_erase

Erase all saved configuration (factory reset).

```c
void flash_memory_erase(void);
```

After erasing, `flash_memory_read()` will return `false` and populate defaults.

---

## WiFi Management

### wifi_connect

Connect to a WiFi network with visual feedback.

```c
ReturnCode wifi_connect(const char* ssid, const char* password, Matrix* mtrx);
```

**Parameters:**

- `ssid` - Network SSID
- `password` - Network password
- `mtrx` - Matrix display for status feedback

**Returns:**

- `RC_SUCCESS` on success
- Error code on failure (timeout or connection error)

**Behavior:**

- Blocks until connected or timeout
- Displays "WIFI..." with animated dots during connection
- Shows "TIMEOUT" error message on failure
- Timeout configured by `MAX_WIFI_CONNECTION_ATTEMPTS` in `const.h` (default: 100 attempts x 100ms = 10 seconds)

**Example:**

```c
device_config_t config;
flash_memory_read(&config);
ReturnCode rc = wifi_connect(config.ssid, config.password, &matrix);
if (RC_SUCCESS(rc)) {
    // Connected
}
```

---

### wifi_disconnect

Disconnect from WiFi network.

```c
void wifi_disconnect(void);
```

---

### wifi_is_connected

Check if WiFi is currently connected.

```c
bool wifi_is_connected(void);
```

---

## HTTP Requests

### http_get

Send an HTTP GET request with visual feedback.

```c
typedef void (*http_response_callback_t)(const char* body, size_t len, bool complete);

ReturnCode http_get(const char* host,
                    uint16_t port,
                    const char* path,
                    http_response_callback_t callback,
                    Matrix* mtrx);
```

**Parameters:**

- `host` - Hostname (e.g., `"api.example.com"`)
- `port` - Port number (e.g., `80`)
- `path` - Request path (e.g., `"/v1/weather?lat=51.5&lon=0.1"`)
- `callback` - Function called with response data
- `mtrx` - Matrix display for status feedback

**Callback Parameters:**

- `body` - Response body content
- `len` - Length of body
- `complete` - `true` if this is the final callback with complete data

**Behavior:**

- Blocks until response received or timeout (30 seconds)
- Displays "DATA..." with animated dots while waiting
- Shows "TIMEOUT" error message on failure
- Final callback has `complete = true`

**Example:**

```c
void handle_response(const char* body, size_t len, bool complete) {
    if (!complete) return;
    float temp = parse_temperature(body);
}

ReturnCode rc = http_get("api.open-meteo.com", 80,
    "/v1/forecast?latitude=51.5&longitude=-0.1",
    handle_response, &matrix);
```

---

**Next:** [Building a Mode Tutorial](../building-a-mode.md) →
