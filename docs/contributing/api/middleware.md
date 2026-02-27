# Middleware API

The Middleware API provides WiFi connectivity and HTTP request handling for fetching remote data.

## WiFi Management

### wifi_join

Connect to a WiFi network (blocking).

```c
int wifi_join(const char* ssid, const char* password);
```

**Parameters:**

- `ssid` - Network SSID
- `password` - Network password

**Returns:**

- `0` on success
- Non-zero error code on failure

Blocks until connected. Use `wifi_join_async()` for non-blocking.

---

### wifi_join_async

Connect to WiFi without blocking (recommended).

```c
int wifi_join_async(const char* ssid, const char* password);
```

**Returns:** `0` if started, non-zero on error

---

### wifi_disconnect

Disconnect from WiFi network.

```c
void wifi_disconnect(void);
```

**Example:**

```c
wifi_disconnect();
printf("WiFi disconnected\n");
```

---

## Connection Manager

For robust WiFi handling with retries and visual feedback.

### WifiConnectionManager

Connection state tracker with retry logic.

```c
typedef struct {
    int max_attempts;        // Max connection attempts
    int attempts;            // Current attempt count
    bool is_connecting;      // Currently attempting connection
    bool has_failed;         // Connection failed
} WifiConnectionManager;
```

---

### wifi_manager_init

Initialize connection manager.

```c
void wifi_manager_init(WifiConnectionManager* mgr, int max_attempts);
```

**Parameters:**

- `mgr` - Manager struct
- `max_attempts` - Maximum connection retry attempts

**Example:**

```c
WifiConnectionManager mgr;
wifi_manager_init(&mgr, 100);  // Retry up to 100 times
```

---

### wifi_manager_start

Start async connection with retry logic.

```c
void wifi_manager_start(WifiConnectionManager* mgr,
                       const char* ssid,
                       const char* password);
```

**Example:**

```c
wifi_manager_start(&mgr, WIFI_SSID, WIFI_PASSWORD);

while (wifi_manager_is_connecting(&mgr)) {
    display_connecting_animation();
    sleep_ms(500);
}

if (wifi_manager_has_failed(&mgr)) {
    display_error_message();
}
```

---

### wifi_manager_is_connecting

Check if connection attempt is in progress.

```c
bool wifi_manager_is_connecting(const WifiConnectionManager* mgr);
```

**Returns:** `true` if currently attempting to connect

---

### wifi_manager_has_failed

Check if all connection attempts failed.

```c
bool wifi_manager_has_failed(const WifiConnectionManager* mgr);
```

**Returns:** `true` if max attempts reached without success

---

### wifi_manager_get_attempts

Get current attempt count.

```c
int wifi_manager_get_attempts(const WifiConnectionManager* mgr);
```

**Returns:** Number of connection attempts made

**Example:**

```c
int attempts = wifi_manager_get_attempts(&mgr);
printf("Connection attempt %d/%d\n", attempts, mgr.max_attempts);
```

---

### wifi_manager_reset

Reset manager state for retry.

```c
void wifi_manager_reset(WifiConnectionManager* mgr);
```

**Example:**

```c
if (wifi_manager_has_failed(&mgr)) {
    printf("Retrying connection...\n");
    wifi_manager_reset(&mgr);
    wifi_manager_start(&mgr, WIFI_SSID, WIFI_PASSWORD);
}
```

---

### wifi_append_connecting_dots

Animated "connecting..." helper for UI feedback.

```c
void wifi_append_connecting_dots(char* buffer, size_t buffer_size, int* dot_counter);
```

**Parameters:**

- `buffer` - String buffer (e.g., "CONNECTING")
- `buffer_size` - Buffer capacity
- `dot_counter` - Pointer to dot count (0-3)

**Behavior:** Appends 0-3 dots to buffer, cycling automatically

**Example:**

```c
char status[32] = "CONNECTING";
int dots = 0;

while (wifi_manager_is_connecting(&mgr)) {
    wifi_append_connecting_dots(status, sizeof(status), &dots);
    matrix_clear(&mtrx);
    matrix_display_word(status, 0, 1, &ORANGE);
    matrix_show(&mtrx);
    sleep_ms(500);

    // Reset for next iteration
    strcpy(status, "CONNECTING");
}
```

---

## HTTP Requests

### http_get

Perform an HTTP GET request (blocking).

```c
typedef void (*http_response_callback)(const char* response, size_t length);

int http_get(const char* host_url,
             const char* url_sub_path,
             http_response_callback callback);
```

**Parameters:**

- `host_url` - Full hostname with protocol (e.g., `"http://api.example.com"`)
- `url_sub_path` - Path after host (e.g., `"/weather/current"`)
- `callback` - Function called with response data

**Returns:** `0` on success, non-zero on error

**Example:**

```c
void handle_response(const char* response, size_t length) {
    printf("Received %zu bytes:\n%s\n", length, response);

    // Parse JSON, extract data, etc.
}

int result = http_get(
    "http://api.openweathermap.org",
    "/data/2.5/weather?q=London&appid=YOUR_KEY",
    handle_response
);

if (result != 0) {
    printf("HTTP request failed: %d\n", result);
}
```

---

## Example

```c
// Connect with visual feedback
WifiConnectionManager mgr;
wifi_manager_init(&mgr, 100);
wifi_manager_start(&mgr, WIFI_SSID, WIFI_PASSWORD);

while (wifi_manager_is_connecting(&mgr)) {
    display_connecting_animation();
    sleep_ms(500);
}

// Make HTTP request
http_get("http://api.example.com", "/data", callback);
```

---

**Next:** [Building a Mode Tutorial](../building-a-mode.md) →
