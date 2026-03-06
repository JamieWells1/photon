# Mode API

Display modes are the core features of Photon. This API defines how modes are structured and integrated into the menu system.

## Mode Structure

### Menu Types

#### Mode

Top-level menu categories.

```c
typedef enum {
    MENU_CRYPTO = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
    MENU_CONFIG = 3,
    MENU_RESET = 4,
} Mode;
```

---

### Menu Structs

#### Menu

Represents a main menu item.

```c
typedef struct {
    const Mode mode;      // Mode identifier
    const char* text;     // Display text
    const Glyph* icon;    // Icon graphic
} Menu;
```

**Example:**

```c
static const Menu MENUS[] = {
    {MENU_CRYPTO, "CRYPTO", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
    {MENU_CONFIG, "CONFIG", &ICONS_ARR[DATA]},
    {MENU_RESET, "RESET", &ICONS_ARR[POWER]},
};
```

---

#### MenuState

Tracks current menu selection.

```c
typedef struct {
    Mode mode;  // Currently selected mode
} MenuState;
```

---

## Mode Entry Point

### menu_start

Initialize and run the menu system.

```c
void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);
```

**Parameters:**

- `btns` - Array of input buttons
- `rtr` - Rotary encoder
- `mtrx` - LED matrix

**Behavior:**

- Called each iteration of the main loop
- Handles menu navigation, mode display, and input detection
- Rotary encoder scrolls between modes with slide animations
- Button press enters the selected mode

---

## Mode Implementations

Each display mode implements a display function and optionally a cleanup function.

### Ticker Mode

```c
void ticker_display(Matrix* mtrx, int* offset);
void ticker_cleanup(void);
```

**Purpose:** Display live cryptocurrency prices (Bitcoin, Ethereum, Solana) with 24-hour change percentage. Fetches from Kraken API every 15 minutes. Scrolls continuously across the display.

---

### Game Mode

```c
void game_display(Matrix* mtrx);
```

**Purpose:** Game implementations (currently a stub/TODO).

---

### Weather Mode

```c
void weather_display(Matrix* mtrx, int* hour_offset_from_now_to_display);
void weather_cleanup(void);
```

**Purpose:** Display weather information from Open-Meteo API. Shows temperature with dynamic color (blue for cold, green for moderate, red for hot), weather icons (sunny, cloudy, rain, snow, lightning, moon), and hour label. Rotary encoder scrolls through 48-hour forecast.

**Additional Functions:**

```c
int weather_current_hour_index();  // Get current hour offset
int weather_hours_fetched();       // Get number of hours available
```

---

### Config Mode

```c
void config_display(Matrix* mtrx);
void config_cleanup(void);
```

**Purpose:** Start a web server at `http://photon.local` for device configuration. The matrix scrolls the URL to guide the user. The web interface allows changing:

- **Brightness** (1-25%)
- **Location** (18 cities worldwide)
- **Currency** (12 currencies)

Saves settings to flash and reboots the device.

**Behavior:**

- Reconnects WiFi if disconnected
- Starts mDNS so `photon.local` resolves
- Starts HTTP server on port 80
- Displays scrolling URL on the matrix
- On save, writes config to flash and reboots

---

### Reset Mode

```c
// Handled internally by menu.c (no separate module)
```

**Purpose:** Factory reset the device by erasing all saved configuration from flash memory. Displays a YES/NO confirmation prompt.

**Behavior:**

- Waits for all buttons to be released before accepting input (prevents accidental triggers)
- Displays "YES" (green) and "NO" (red) with an underline on the current selection
- Rotary encoder switches between YES and NO
- Button press confirms selection:
  - **YES**: Erases flash, displays "DONE", reboots device
  - **NO**: Returns to main menu

---

## Adding a Mode

See [Building a Mode](../building-a-mode.md) for step-by-step guide.

---

## Tips

- Keep logic under 50ms per frame
- Use `static` variables for persistent state
- Implement a `cleanup()` function to reset state when exiting your mode
- See existing modes in `src/modes/` for examples

---

**Next:** [Middleware API](middleware.md) →
