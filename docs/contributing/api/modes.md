# Mode API

Display modes are the core features of Photon. This API defines how modes are structured and integrated into the menu system.

## Mode Structure

### Menu Types

#### Mode

Top-level menu categories.

```c
typedef enum {
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
    MENU_RESET = 3,
} Mode;
```

#### Mode Ranges

Mode ranges define the start positions for each mode's submodes.

```c
#define TKR_START 0
#define TKR_END 3
#define GAME_START (TKR_START + TKR_END)
#define GAME_END 1
#define TEMP_START (GAME_START + GAME_END)
#define TEMP_END 2
#define RESET_START (TEMP_START + TEMP_END)
#define RESET_END 1
```

**Mode ranges:**

- Crypto Tickers: 0-2 (3 submodes)
- Games: 3 (1 submode)
- Weather: 4-5 (2 submodes)
- Reset: 6 (1 submode)

---

### Menu Structs

#### Menu

Represents a main menu item.

```c
typedef struct {
    const Mode mode;  // Mode identifier
    const char* text;     // Display text
    const Glyph* icon;    // Icon graphic
} Menu;
```

**Example:**

```c
static const Menu MENUS[] = {
    {MENU_TICKERS, "CRYPTO", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
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

**Notes:**

- Submodes are managed internally by each mode implementation
- The menu system uses mode ranges to determine which mode handles input

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

- Infinite loop handling menu navigation and mode display
- Never returns (this is your application main loop)

**Typical usage:**

```c
int main() {
    // Initialize hardware...
    Button btns[2] = {/* ... */};
    Rotator rtr = {/* ... */};
    Matrix mtrx = {/* ... */};

    // Start menu (never returns)
    menu_start(btns, &rtr, &mtrx);
}
```

---

## Mode Implementation

Each display mode must implement a display function matching this signature:

### Crypto Mode

```c
void ticker_display(Matrix* mtrx, int* ticker_offset);
```

**Purpose:** Display cryptocurrency ticker information

**Parameters:**

- `mtrx` - LED matrix
- `ticker_offset` - Pointer to offset for cycling through tickers

**Current Implementation:**

- XBT (Bitcoin), ETH (Ethereum), SOL (Solana)
- Fetches live prices from exchange API
- Displays current price and 24-hour percentage change
- Rotary encoder cycles through different cryptocurrencies

**Additional Functions:**

```c
void ticker_cleanup(void);  // Free ticker data
```

---

### Game Mode

```c
void game_display(Matrix* mtrx);
```

**Purpose:** Dispatch to specific game implementations

**Current Implementation:**

- Tetris game with rotary encoder and button controls

---

### Weather Mode

```c
void weather_display(Matrix* mtrx, int* hour_to_display);
```

**Purpose:** Display weather information

**Parameters:**

- `mtrx` - LED matrix
- `hour_to_display` - Pointer to hour index (for hourly mode)

**Additional Functions:**

```c
int weather_current_hour_index();  // Get current hour offset
int weather_hours_fetched();       // Get number of hours available
void weather_cleanup(void);        // Free weather data
```

---

### Reset Mode

The reset mode is handled internally by the menu system and does not require a separate display function. It provides a user interface for factory resetting WiFi credentials.

**Functionality:**

- Displays "YES" and "NO" options
- Allows user to select with rotary encoder
- Confirms selection with button press
- Erases flash config and reboots on "YES"
- Returns to main menu on "NO"

---

## Adding a Mode

See [Building a Mode](../building-a-mode.md) for step-by-step guide.

---

## Tips

- Keep logic under 50ms per frame
- Use `static` variables for persistent state
- See existing modes in `src/modes/` for examples

---

**Next:** [Middleware API](middleware.md) →
