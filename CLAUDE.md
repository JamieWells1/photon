# CLAUDE.md — Photon Firmware

This file provides guidance for AI assistants working in this repository.

## Project Overview

**Photon** is embedded C firmware for the **Raspberry Pi Pico 2 W** microcontroller. It drives a **32×8 WS2812B RGB LED matrix** as an interactive display with WiFi connectivity. Features include a cryptocurrency ticker, weather display, a Tetris game, and a rotary-encoder-driven menu system.

The codebase targets **C11** with the **Pico SDK 2.2.0** and uses **CMake + Ninja** as the build system.

---

## Repository Layout

```
photon/
├── main.c                  # Entry point and main loop
├── CMakeLists.txt          # Top-level CMake build configuration
├── lwipopts.h              # lwIP networking stack tuning
├── src/
│   ├── common/             # Shared constants, macros, location data, secrets
│   │   ├── const.h         # Hardware pins, WiFi settings, API hostnames
│   │   ├── location.h/.c   # Location/timezone data
│   │   ├── macro.h         # Convenience macros
│   │   └── secrets_config.h# WiFi/secrets template (do NOT commit real secrets)
│   ├── display/            # LED matrix rendering engine
│   │   ├── matrix.h/.c     # Core pixel buffer and DMA-driven WS2812 output
│   │   ├── graphics.h/.c   # Drawing primitives (scrolling text, shapes)
│   │   └── glyphs.h/.c     # Pre-computed bitmap font and icon library
│   ├── input/              # Physical controls
│   │   └── input.h/.c      # Rotary encoder (quadrature) + button debouncing
│   ├── middleware/         # Network abstractions
│   │   ├── wifi.h/.c       # Connect/disconnect helpers (power-aware)
│   │   └── http.h/.c       # Callback-based HTTP client
│   ├── modes/              # Display mode implementations
│   │   ├── ticker.h/.c     # Cryptocurrency price ticker (Kraken API)
│   │   ├── weather.h/.c    # Weather display (Open-Meteo API)
│   │   ├── game.h/.c       # Game mode dispatcher
│   │   └── games/
│   │       └── tetris.h/.c # Tetris on the LED matrix
│   ├── config.h/.c         # Runtime configuration (brightness, location, currency)
│   ├── global.h/.c         # Global state (MENU_STATE, TKR_ARR, etc.)
│   ├── menu.h/.c           # Menu navigation state machine
│   └── utils.h/.c          # Misc utilities
├── pio/
│   └── ws2812.pio          # PIO assembly for bit-perfect WS2812 protocol
├── tests/
│   └── test_example.c      # Unity-based unit tests (built separately)
├── proxy/
│   ├── https_proxy.py      # Python HTTPS→HTTP proxy for Pico
│   └── const.py            # Proxy configuration constants
├── docs/                   # MkDocs Material documentation site
│   ├── getting-started/    # Hardware wiring, flashing, configuration
│   ├── contributing/       # Contributor guide + API references
│   └── progress/           # Build progress log
├── .github/workflows/
│   ├── push.yml            # Build firmware on every push
│   ├── docs.yml            # Deploy docs to GitHub Pages (main only)
│   └── release.yml         # AI-assisted release notes + UF2 artifact on tags
└── mkdocs.yml              # Documentation site configuration
```

---

## Build System

### Prerequisites

- CMake ≥ 3.13
- ARM GCC toolchain: `arm-none-eabi-gcc`
- Pico SDK 2.2.0 (set `PICO_SDK_PATH` env var, or let CMake fetch it)
- Ninja (recommended) or GNU Make

### Build Commands

```bash
# Configure (targeting Pico 2 W)
cmake -B build -DPICO_BOARD=pico2_w

# Compile (parallel)
cmake --build build -- -j4

# Output firmware image
ls build/photon.uf2
```

### Flashing

Hold **BOOTSEL** on the Pico, connect via USB — it mounts as a mass-storage device. Copy `photon.uf2` onto it.

### Secrets

Copy `src/common/secrets.example.h` → `src/common/secrets.h` and fill in WiFi credentials. **Never commit `secrets.h`.**

---

## Testing

Framework: **Unity** (fetched automatically by CMake via `FetchContent`).

```bash
cmake -B build
cmake --build build
./build/test_example
```

Tests live in `tests/`. Hardware-dependent code (GPIO, PIO, networking) cannot be tested without a physical Pico board; unit tests cover pure-logic functions only.

When adding new logic, prefer writing it in a way that is testable (free of hardware calls) and add a corresponding test in `tests/`.

---

## Development Workflows

### Adding a New Display Mode

1. Create `src/modes/my_mode.h` and `src/modes/my_mode.c`.
2. Implement the standard mode interface — see `docs/contributing/building-a-mode.md` for the required function signatures.
3. Register the mode in `src/menu.c` and `src/global.h`.
4. Add it to `CMakeLists.txt` (source list + any libraries).
5. Document it under `docs/contributing/api/`.

### Modifying the Display

- All pixel writes go through `matrix.h` — use `matrix_set_pixel()` and related primitives.
- `graphics.h` provides higher-level helpers (scrolling text, horizontal/vertical lines).
- Glyphs are pre-computed bitmaps in `glyphs.h`; add new icons there as `Glyph` structs.
- PIO handles the WS2812 timing — do not bit-bang the LED data pin directly.

### Networking

- WiFi is connected on-demand and disconnected after use to save power.
- HTTP responses are handled via `http_response_callback_t` callbacks (non-blocking).
- The Pico cannot make native HTTPS requests — route them through the proxy in `proxy/https_proxy.py`.

### Documentation

```bash
pip install -r requirements.txt
mkdocs serve          # live preview at http://127.0.0.1:8000
mkdocs build --strict # production build (fails on warnings)
```

Docs deploy automatically to GitHub Pages on push to `main`.

---

## Code Conventions

### Language & Standard

- **C11** for all firmware source.
- **C++17** is enabled in CMake (for Pico SDK compatibility) but avoid C++ in new code.

### Naming

| Construct | Convention | Example |
|-----------|-----------|---------|
| Files | `snake_case` | `matrix.c`, `http.h` |
| Functions | `subsystem_verb_noun` | `matrix_set_pixel`, `wifi_connect` |
| Types (typedef structs/enums) | `PascalCase` | `Button`, `Rotator`, `MenuItem` |
| Constants / macros | `UPPER_SNAKE_CASE` | `MATRIX_WIDTH`, `LED_PIN` |
| Global state variables | `UPPER_SNAKE_CASE` | `MENU_STATE`, `TKR_ARR` |

### Style

- 4-space indentation (no tabs).
- Opening braces on the same line as the control statement.
- Keep functions short and single-purpose.
- Declare variables at the top of their scope (C89 habit is fine in C11 but not required).
- Prefer `const` for data that will not change.
- No dynamic memory allocation (`malloc`/`free`) — use static or stack allocation.

### Compiler Warnings

The build enables `-Wall -Wextra -Werror=return-type -Wno-unused-parameter`. Fix all warnings; do not suppress them without a comment explaining why.

### Clangd / Static Analysis

A `.clangd` file at the root configures clangd to use the ARM GCC toolchain. After running CMake, a `compile_commands.json` is generated in `build/` — clangd reads this for accurate diagnostics.

---

## CI/CD

| Workflow | Trigger | What it does |
|----------|---------|-------------|
| `push.yml` | Every push | Installs ARM GCC, caches Pico SDK, builds firmware, uploads UF2 artifact |
| `docs.yml` | Push to `main` or manual | Builds MkDocs site (strict), deploys to GitHub Pages |
| `release.yml` | Push of tag `v*` | Generates AI release notes (OpenAI), builds firmware, creates GitHub Release with UF2 |

Ensure the build passes before merging. The firmware build is the primary gate.

---

## Architecture Notes

### Global State

`src/global.h` holds shared mutable state (current menu position, ticker data array). This is intentional for embedded code — avoid adding new globals unless necessary; prefer passing state through function parameters or callbacks.

### Callback-Based HTTP

```c
// Typical pattern:
http_get(host, path, port, my_callback, user_data);

void my_callback(const char *body, size_t len, void *user_data) {
    // parse response
}
```

Callbacks run on the lwIP thread — keep them short; offload heavy parsing to the main loop if needed.

### PIO for WS2812

`pio/ws2812.pio` is compiled at build time into a C header. The matrix driver loads this program into a PIO state machine — this frees the CPU from bit-timing the LED protocol.

### Power Management

WiFi is disconnected after each data fetch. Do not leave it connected in the background unless you have a specific reason; add a comment if you do.

---

## Key Files Quick Reference

| File | Why you might edit it |
|------|-----------------------|
| `src/common/const.h` | Change pin assignments, API hostnames, WiFi config |
| `src/config.h/.c` | Add new user-configurable settings |
| `src/display/glyphs.h/.c` | Add new icons or characters |
| `src/display/graphics.h/.c` | Add new drawing primitives |
| `src/menu.h/.c` | Add or rearrange menu items |
| `src/global.h/.c` | Add global state (use sparingly) |
| `CMakeLists.txt` | Add source files, libraries, or compiler flags |
| `mkdocs.yml` | Add or reorganise documentation pages |
