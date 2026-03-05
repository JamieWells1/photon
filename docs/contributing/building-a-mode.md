# Building a Mode from Scratch

This guide shows you how to add a new display mode to Photon.

## Step 1: Define Your Mode

Edit [`src/menu.h`](https://github.com/JamieWells1/photon/blob/main/src/menu.h):

```c
typedef enum {
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
    MENU_RESET = 3,
    MENU_YOUR_MODE = 4,  // Add this
} Mode;
```

Update the menu array:

```c
static const Menu MENUS[] = {
    {MENU_TICKERS, "CRYPTO", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
    {MENU_RESET, "RESET", &ICONS_ARR[POWER]},
    {MENU_YOUR_MODE, "NAME", &ICONS_ARR[YOUR_ICON]},  // Add this
};
```

Update range constants:

```c
#define TKR_START 0
#define TKR_END 3
#define GAME_START (TKR_START + TKR_END)
#define GAME_END 1
#define TEMP_START (GAME_START + GAME_END)
#define TEMP_END 2
#define RESET_START (TEMP_START + TEMP_END)
#define RESET_END 1
#define YOUR_START (RESET_START + RESET_END)  // Add this
#define YOUR_END 1                             // Number of submodes
```

---

## Step 2: Create Mode Files

Create `src/modes/yourmode.h`:

```c
#ifndef YOURMODE_H
#define YOURMODE_H

#include <matrix.h>

void yourmode_display(Matrix* mtrx);

#endif
```

Create `src/modes/yourmode.c`:

```c
#include <yourmode.h>
#include <glyphs.h>
#include <graphics.h>

void yourmode_display(Matrix* mtrx) {
    static bool initialized = false;

    if (!initialized) {
        // First-time initialization
        initialized = true;
    }

    // Your implementation here
    // Example: Display static text
    matrix_clear(mtrx);
    matrix_display_word("HELLO", 5, 1, &GREEN);
    matrix_show(mtrx);
}
```

---

## Step 3: Integrate into Menu

Edit [`src/menu.c`](https://github.com/JamieWells1/photon/blob/main/src/menu.c):

Add include:

```c
#include <yourmode.h>
```

Add handling in `menu_enter_sub_menu`:

```c
static void menu_enter_sub_menu(Menu menu_state, Button* btns, Rotator* rtr, Matrix* mtrx)
{
    Mode target_menu;

    if (menu_state.mode == MENU_TICKERS) {
        target_menu = TKR_START;
    }
    // ... existing cases ...
    else if (menu_state.mode == MENU_YOUR_MODE) {
        target_menu = YOUR_START;  // Add this
    }
    else {
        return;
    }

    // ... rest of function ...

    if (g_current_mode == YOUR_START) {
        yourmode_display(mtrx);  // Add this
    }
}
```

Add input handling in `menu_detect_inputs`:

```c
if (g_in_submenu) {
    // ... existing mode handlers ...
    if (g_current_mode == YOUR_START) {
        menu_sub_menu_yourmode_detect_inputs(btns, rtr, mtrx);  // Add this
    }
}
```

Add input handler function:

```c
static void menu_sub_menu_yourmode_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (input_btn_pressed(&btns[0])) {
        menu_reset_states(mtrx);  // Exit mode on left button
    }

    // Add your custom input handling here
}
```

---

## Step 4: Update Build

Edit [`CMakeLists.txt`](https://github.com/JamieWells1/photon/blob/main/CMakeLists.txt):

```cmake
set(MODE_SOURCES
    # ... existing sources
    src/modes/yourmode.c  # Add this
)
```

---

## Step 5: Build and Test

```bash
cmake --build build -- -j4
picotool load build/photon.uf2 -fx
```

---

## Example Implementations

### Static Display

```c
void info_display(Matrix* mtrx) {
    matrix_clear(mtrx);
    matrix_display_word("INFO", 8, 1, &BLUE);
    matrix_show(mtrx);
}
```

### Animated Display

```c
void wave_display(Matrix* mtrx) {
    static int offset = 0;

    matrix_clear(mtrx);
    for (int x = 0; x < MATRIX_WIDTH; x++) {
        int y = 4 + 3 * sin((x + offset) * 0.3);
        Pixel p = {x, y, {0, 150, 255}};
        matrix_set_pixel(&p);
    }
    matrix_show(mtrx);
    offset++;
}
```

### With State Management

```c
void counter_display(Matrix* mtrx) {
    static int count = 0;
    static bool initialized = false;

    if (!initialized) {
        count = 0;
        initialized = true;
    }

    // Input handling is done in menu_sub_menu_counter_detect_inputs()
    // This function just displays the current count

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", count);

    matrix_clear(mtrx);
    matrix_display_word(buffer, 10, 1, &GREEN);
    matrix_show(mtrx);
}
```

**Note:** Input handling is done separately in the `menu_sub_menu_yourmode_detect_inputs()` function in `menu.c`, not in the display function.

---

## Tips

- Use `static` variables to preserve state between frames
- Keep logic under 50ms to maintain frame rate
- See existing modes in `src/modes/` for reference
- Check the [API Reference](api/display.md) for available functions

---

**Next:** [API Reference](api/display.md) | [Contributing Guide](index.md)
