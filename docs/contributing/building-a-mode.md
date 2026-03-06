# Building a Mode from Scratch

This guide shows you how to add a new display mode to Photon.

## Step 1: Define Your Mode

Edit [`src/menu.h`](https://github.com/JamieWells1/photon/blob/main/src/menu.h):

```c
typedef enum {
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
    MENU_CONFIG = 3,
    MENU_RESET = 4,
    MENU_YOUR_MODE = 5,  // Add this
} Mode;
```

Update the menu array:

```c
static const Menu MENUS[] = {
    // ... existing menus
    {MENU_YOUR_MODE, "NAME", &ICONS_ARR[YOUR_ICON]},  // Add this
};
```

Update range constants:

```c
#define YOUR_START (RESET_START + RESET_END)
#define YOUR_END 1
```

---

## Step 2: Create Mode Files

Create `src/modes/yourmode.h`:

```c
#ifndef YOURMODE_H
#define YOURMODE_H

#include <matrix.h>

void yourmode_display(Matrix* mtrx);
void yourmode_cleanup(void);

#endif
```

Create `src/modes/yourmode.c`:

```c
#include <yourmode.h>
#include <glyphs.h>

void yourmode_display(Matrix* mtrx) {
    matrix_clear(mtrx);
    matrix_display_word("HELLO", 5, 1, &GREEN);
    matrix_show(mtrx);
}

void yourmode_cleanup(void) {
    // Reset any static state
}
```

---

## Step 3: Integrate into Menu

Edit [`src/menu.c`](https://github.com/JamieWells1/photon/blob/main/src/menu.c):

Add include:

```c
#include <yourmode.h>
```

Add display call in `menu_enter_sub_menu()`:

```c
else if (g_current_mode == YOUR_START)
{
    yourmode_display(mtrx);
}
```

Add cleanup call in `menu_reset_states()`:

```c
yourmode_cleanup();
```

Add input handling in `menu_detect_inputs()`:

```c
if (g_current_mode == YOUR_START)
{
    // Handle inputs for your mode
    if (input_btn_pressed(&btns[0]))
    {
        menu_reset_states(mtrx);  // Exit to main menu
    }
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

### With Data Fetching

```c
void mymode_display(Matrix* mtrx) {
    device_config_t config;
    flash_memory_read(&config);  // Defaults applied automatically on failure

    // Use config.location, config.currency, etc.
}
```

### With Input

```c
// Input is handled in menu.c, not in the display function.
// Your mode gets called each frame via menu_enter_sub_menu().
// Use rotary encoder state passed through static variables.
```

---

## Tips

- Use `static` variables to preserve state between frames
- Keep logic under 50ms to maintain frame rate
- Implement a `cleanup()` function to reset state when exiting
- `flash_memory_read()` always returns usable defaults, no error handling needed for settings
- See existing modes in `src/modes/` for reference
- Check the [API Reference](api/display.md) for available functions

---

**Next:** [API Reference](api/display.md) | [Contributing Guide](index.md)
