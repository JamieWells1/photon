# Building a Mode from Scratch

This guide shows you how to add a new display mode to Photon.

## Step 1: Define Your Mode

Edit [`src/menu.h`](https://github.com/JamieWells1/photon/blob/main/src/menu.h):

```c
typedef enum {
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
    MENU_YOUR_MODE = 3,  // Add this
} MainMode;

typedef enum {
    // ... existing modes
    YOUR_SUBMODE = 6,  // Add this
} SubMode;
```

Update the menu arrays:

```c
static const Menu MENUS[4] = {  // Increase array size
    // ... existing menus
    {MENU_YOUR_MODE, "NAME", &ICONS_ARR[YOUR_ICON]},  // Add this
};

static const SubMenu SUB_MENUS[7] = {  // Increase array size
    // ... existing submenus
    {YOUR_SUBMODE, "SUBNAME", NULL},  // Add this
};
```

Update range constants:

```c
#define YOUR_START (TEMP_START + TEMP_COUNT)
#define YOUR_COUNT 1
```

---

## Step 2: Create Mode Files

Create `src/modes/yourmode.h`:

```c
#ifndef YOURMODE_H
#define YOURMODE_H

#include <menu.h>
#include <matrix.h>
#include <input.h>

void yourmode_display(SubMenu sub_mode, Matrix* mtrx, Button* btns, Rotator* rtr);

#endif
```

Create `src/modes/yourmode.c`:

```c
#include <yourmode.h>
#include <glyphs.h>

void yourmode_display(SubMenu sub_mode, Matrix* mtrx, Button* btns, Rotator* rtr) {
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

Add case to switch statement:

```c
switch (state.main_mode) {
    // ... existing cases

    case MENU_YOUR_MODE:
        yourmode_display(SUB_MENUS[state.sub_mode], mtrx, btns, rtr);
        break;
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
void info_display(SubMenu sub_mode, Matrix* mtrx, Button* btns, Rotator* rtr) {
    matrix_clear(mtrx);
    matrix_display_word("INFO", 8, 1, &BLUE);
    matrix_show(mtrx);
}
```

### Animated Display

```c
void wave_display(SubMenu sub_mode, Matrix* mtrx, Button* btns, Rotator* rtr) {
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

### With Input

```c
void counter_display(SubMenu sub_mode, Matrix* mtrx, Button* btns, Rotator* rtr) {
    static int count = 0;

    if (input_rtr_cw(rtr)) count++;
    if (input_rtr_anti_cw(rtr)) count--;

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", count);

    matrix_clear(mtrx);
    matrix_display_word(buffer, 10, 1, &GREEN);
    matrix_show(mtrx);
}
```

---

## Tips

- Use `static` variables to preserve state between frames
- Keep logic under 50ms to maintain frame rate
- See existing modes in `src/modes/` for reference
- Check the [API Reference](api/display.md) for available functions

---

**Next:** [API Reference](api/display.md) | [Contributing Guide](index.md)
