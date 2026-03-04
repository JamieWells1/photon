# Flash Storage + Captive Portal Implementation Plan

## Goal
Transform Pico from hardcoded WiFi to plug-and-play captive portal credential provisioning with persistent flash storage.

## Architecture Overview

```
Boot → Check Flash → [No Creds] → AP Mode + Captive Portal → Save to Flash → Reboot
                   → [Has Creds] → STA Mode → Connect → Normal Operation
```

## Implementation Steps

### 1. Flash Storage Layer (`src/middleware/flash_config.{h,c}`)

**Dependencies**: `pico_flash`, `hardware_flash`, `hardware_sync`

**Structure**:
```c
typedef struct {
    uint32_t magic;        // 0x50484F54 ("PHOT")
    char ssid[64];
    char password[64];
    uint32_t crc32;
} wifi_config_t;
```

**Functions**:
- `flash_config_read(wifi_config_t*)` - Read from last flash sector (XIP_BASE + PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
- `flash_config_write(wifi_config_t*)` - Erase + write with interrupts disabled, validate CRC
- `flash_config_exists()` - Check magic + CRC validity
- `flash_config_erase()` - Factory reset capability

**Key Details**:
- Use last 4KB sector to avoid firmware collision
- Calculate CRC32 for integrity validation
- Flash writes require 256-byte alignment, sector erases 4KB
- Wrap operations in `uint32_t ints = save_and_disable_interrupts()` / `restore_interrupts(ints)`

### 2. WiFi AP Mode Extension (`src/middleware/wifi.{h,c}`)

**Add Functions**:
- `wifi_init_ap(const char* ssid, const char* password)` - Enable AP with `cyw43_arch_enable_ap_mode()`, DHCP server auto-enabled
- `wifi_is_ap_mode()` - Track current mode
- Modify `wifi_init()` to support both STA and AP initialization

**AP Configuration**:
- SSID: "Photon-Setup-XXXX" (XXXX = last 4 hex of MAC)
- Password: Open or WPA2 with simple PIN
- IP: 192.168.4.1 (lwIP default for AP)
- Channel: 1

### 3. DNS Server (`src/middleware/dns_server.{h,c}`)

**Purpose**: Redirect all DNS queries to Pico's IP (captive portal trigger)

**Dependencies**: `lwip/udp.h`

**Implementation**:
```c
udp_recv(pcb, dns_recv_callback, NULL);
udp_bind(pcb, IP_ADDR_ANY, 53);
```

**Logic**: Parse DNS query → respond with A record pointing to 192.168.4.1 for all domains

**Packet Structure**:
- Copy query ID + flags (0x8180 = standard response)
- Set answer count = 1
- Echo question section
- Append A record: 192.168.4.1, TTL=60

### 4. HTTP Server (`src/middleware/http_server.{h,c}`)

**Dependencies**: `lwip/tcp.h`

**Routes**:
- `GET /` → Serve HTML form (embedded in C string)
- `POST /save` → Parse `ssid=X&password=Y`, save to flash, respond with success page, trigger reboot after 2s

**Implementation**:
```c
tcp_bind(pcb, IP_ADDR_ANY, 80);
tcp_listen(pcb);
tcp_accept(pcb, http_accept_callback);
```

**HTML Form** (minified, ~500 bytes):
```html
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width">
<title>Photon Setup</title></head><body><h1>WiFi Setup</h1>
<form method="POST" action="/save">
SSID: <input name="ssid" required><br>
Password: <input name="password" type="password"><br>
<button type="submit">Connect</button></form></body></html>
```

**POST Parsing**:
- Find "\r\n\r\n" for body start
- Parse URL-encoded data: `ssid=value&password=value`
- URL decode (replace `+` with space, handle `%XX` encoding)

### 5. Boot Flow Modification (`main.c`)

**New Startup Sequence**:
```c
stdio_init_all();

wifi_config_t config;
if (flash_config_read(&config) && flash_config_exists()) {
    // Normal mode: STA
    if (wifi_connect(config.ssid, config.password, mtrx) == 0) {
        // Success - continue to menu
    } else {
        // Connection failed - show error, enter setup mode after 5s
        flash_config_erase();
        watchdog_reboot(0, 0, 0);
    }
} else {
    // Setup mode: AP + DNS + HTTP
    matrix_display_word_icon_pair("SETUP", &BLUE, NULL, 0);

    wifi_init_ap("Photon-Setup", "");
    dns_server_start();
    http_server_start();

    // Poll until credentials saved (http_server sets flag)
    while (!credentials_saved) {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    watchdog_reboot(0, 0, 0);  // Reboot to apply
}
```

### 6. CMake Integration

**Add to `CMakeLists.txt`**:
```cmake
set(MIDDLEWARE_SOURCES
    src/middleware/wifi.c
    src/middleware/http.c
    src/middleware/flash_config.c
    src/middleware/dns_server.c
    src/middleware/http_server.c
)

target_link_libraries(photon
    pico_stdlib
    hardware_pio
    hardware_flash
    hardware_sync
    pico_cyw43_arch_lwip_threadsafe_background
)
```

### 7. lwIP Configuration Updates

**Add to `lwipopts.h`**:
```c
#define LWIP_HTTPD 0         // Custom server, not pico SDK's
#define LWIP_DHCP_SERVER 1   // Built-in DHCP for AP mode
```

## Implementation Order

1. **Flash layer** - Test read/write in isolation with debug prints
2. **AP mode** - Verify AP advertises and clients can connect
3. **DNS server** - Test with `nslookup google.com 192.168.4.1`
4. **HTTP server** - Serve static form, verify in browser
5. **POST handler** - Parse credentials, log but don't save yet
6. **Integration** - Connect flash write → reboot → STA connection
7. **Error handling** - Failed connection fallback, flash corruption recovery

## Testing Strategy

1. Flash empty Pico → verify AP mode starts
2. Connect to AP → verify captive portal appears
3. Submit credentials → verify flash write + reboot
4. Power cycle → verify auto-connect with stored credentials
5. Force bad credentials → verify fallback to setup mode

## Memory Budget

- Flash config: 4KB (1 sector)
- HTML form: ~0.5KB
- DNS buffer: 512B
- HTTP buffers: ~2KB (reuse existing from http.c)
- Total RAM overhead: ~3KB

## Edge Cases

- **Flash wear**: ~100k write cycles - acceptable for one-time setup
- **Concurrent DNS/HTTP**: lwIP handles via callbacks
- **Timeout**: 5min AP timeout → reboot to retry
- **UTF-8 SSIDs**: Support up to 63 bytes (WiFi spec limit)
- **URL encoding**: Handle special chars in password (`&`, `=`, `%`)
- **Factory reset**: Hold button during boot → erase flash → setup mode

## Security Notes

- Credentials stored plaintext in flash (no encryption on RP2350)
- AP mode password optional (convenience vs security tradeoff)
- HTTPS not feasible (no TLS lib, resource constrained)
- Consider WPA2 AP password in production deployments
