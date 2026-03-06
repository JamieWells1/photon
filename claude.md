# Photon

Embedded firmware for Raspberry Pi Pico 2 W that drives a 32x8 WS2812B LED matrix display with WiFi connectivity.

## Build

```bash
cmake --build build -- -j4
picotool load build/photon.uf2 -fx
```

Cross-compiles for RP2350 (ARM Cortex-M33). Requires Pico SDK. Cannot run tests on host machine -- test on hardware.

## Architecture

```
main.c                    # Init hardware, WiFi connect or captive portal, main loop
src/
├── common/               # Constants (const.h), error codes (error.h), macros (macro.h)
├── display/
│   ├── matrix.c          # LED matrix driver, pixel/text/icon rendering, brightness
│   ├── graphics.c        # Start screen, helper graphics
│   └── glyphs.c          # Font bitmaps (A-Z, 0-9, punctuation) and icon definitions
├── input/
│   └── input.c           # Button debouncing, rotary encoder state machine
├── middleware/
│   ├── flash_memory.c    # Persistent config storage (last flash sector, CRC32 validated)
│   ├── wifi.c            # WiFi STA/AP modes, mDNS
│   └── http.c            # HTTP GET client with callback
├── modes/
│   ├── ticker.c          # Crypto prices from Kraken API
│   ├── weather.c         # Weather from Open-Meteo API
│   ├── config.c          # Web config server at photon.local
│   └── game.c            # Game mode (stub)
├── server/
│   ├── http_server.c     # Captive portal + config web interface
│   ├── dns_server.c      # DNS server for captive portal
│   └── dhcpserver.c      # DHCP server for AP mode
└── menu.c                # Menu navigation, mode dispatch, reset mode
```

## Key APIs

### Flash Memory (3 public functions)

```c
bool flash_memory_read(device_config_t* config);   // Returns true if valid, populates defaults on failure
bool flash_memory_write(const device_config_t* config);
void flash_memory_erase(void);
```

`flash_memory_read` always leaves `config` in a usable state -- on failure it zeros the struct and applies defaults (brightness=8%, location=London, currency=GBP).

### Matrix Display

```c
void matrix_set_pixel(Pixel* pxl);
void matrix_display_word(const char* word, int x, int y, const RGB* col);
void matrix_display_icon(const Glyph* icon, int x, int y, const RGB* col);
void matrix_set_brightness(uint8_t brightness_pct);
void matrix_show(Matrix* mtrx);   // Push buffer to LEDs
void matrix_clear(Matrix* mtrx);  // Zero the buffer
```

All rendering is buffered. Call `matrix_show()` to push to hardware.

## Conventions

- Functions: `module_action_object()` (e.g., `matrix_display_word`)
- Structs: `PascalCase` (e.g., `Matrix`, `Button`)
- Constants: `UPPER_SNAKE_CASE`
- Static globals: `g_` prefix
- Config struct: `device_config_t` (not `wifi_config_t`)
- Error handling: `ReturnCode` enum with `RC_SUCCESS`/`RC_FAILED` macros
- Debug logging: `debug(LEVEL, fmt, ...)` macro (levels: ERROR, WARN, INFO, DEBUG, TRACE)

## Modes

5 menu modes: CRYPTO, GAMES, TEMP, CONFIG, RESET. Each mode implements a `_display()` function called each frame and a `_cleanup()` function called on exit. Reset mode is handled directly in menu.c.
