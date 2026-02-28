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
