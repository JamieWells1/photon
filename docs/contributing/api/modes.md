# Mode API

Display modes are the core features of Photon. This API defines how modes are structured and integrated into the menu system.

## Mode Structure

### Menu Types

#### MainMode

Top-level menu categories.

```c
typedef enum {
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
} MainMode;
```

#### SubMode

Specific items within each main mode.

```c
typedef enum {
    // Ticker submodes (0-2)
    TKR_BTC = 0,
    TKR_XAU,
    TKR_XAG,

    // Game submodes (3)
    GAME_TETRIS,

    // Weather submodes (4-5)
    TEMP_CURRENT,
    TEMP_HOURLY,
} SubMode;
```

---

### Menu Structs

#### Menu

Represents a main menu item.

```c
typedef struct {
    const MainMode mode;  // Mode identifier
    const char* text;     // Display text
    const Glyph* icon;    // Icon graphic
} Menu;
```

**Example:**

```c
static const Menu MENUS[3] = {
    {MENU_TICKERS, "STOCKS", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
};
```

---

#### SubMenu

Represents a submenu item.

```c
typedef struct {
    const SubMode mode;  // Submode identifier
    const char* text;    // Display text
    const Glyph* icon;   // Optional icon (can be NULL)
} SubMenu;
```

**Example:**

```c
static const SubMenu SUB_MENUS[6] = {
    {TKR_BTC, "BTC", NULL},
    {TKR_XAU, "XAU", NULL},
    {TKR_XAG, "XAG", NULL},
    {GAME_TETRIS, "TETRIS", &ICONS_ARR[TETRIS]},
    {TEMP_CURRENT, "NOW", NULL},
    {TEMP_HOURLY, "00", NULL},
};
```

---

#### MenuState

Tracks current menu selection.

```c
typedef struct {
    MainMode main_mode;  // Currently selected main mode
    SubMode sub_mode;    // Currently selected submode
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

### Ticker Mode

```c
void ticker_display(SubMenu sub_mode, Matrix* mtrx);
```

**Purpose:** Display stock/commodity ticker information

**Current Implementation:**

- BTC, XAU (gold), XAG (silver) tickers
- TODO: Fetch live prices via API

---

### Game Mode

```c
void game_display(SubMenu sub_mode, Matrix* mtrx);
```

**Purpose:** Dispatch to specific game implementations

**Current Implementation:**

- Tetris game

---

### Weather Mode

```c
void weather_display(SubMode sub_mode, Matrix* mtrx, int* hour_to_display);
```

**Purpose:** Display weather information

**Parameters:**

- `sub_mode` - `TEMP_CURRENT` or `TEMP_HOURLY`
- `mtrx` - LED matrix
- `hour_to_display` - Pointer to hour index (for hourly mode)

**Additional Functions:**

```c
int weather_current_hour_index();  // Get current hour offset
int weather_hours_fetched();       // Get number of hours available
void weather_cleanup(void);        // Free weather data
```

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
