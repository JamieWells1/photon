# Display API

The Display API provides functions for controlling the LED matrix, rendering text, icons, and graphics.

## Core Types

### RGB

Color representation in 8-bit RGB format.

```c
typedef struct {
    uint8_t r;  // Red (0-255)
    uint8_t g;  // Green (0-255)
    uint8_t b;  // Blue (0-255)
} RGB;
```

**Colors:** `DEFAULT_COLOUR`, `GREEN`, `RED`, `YELLOW`, `LIGHT_BLUE`, `BLUE`, `ORANGE`

### Pixel

Represents a single LED with position and color.

```c
typedef struct {
    uint8_t x;    // X coordinate (0-31)
    uint8_t y;    // Y coordinate (0-7)
    RGB col;      // Color
} Pixel;
```

### Glyph

A bitmap graphic (character, icon, or sprite).

```c
typedef struct {
    const Pixel* pixels;  // Array of pixels
    size_t pxl_count;     // Number of pixels
    int width;            // Width in pixels
    int height;           // Height in pixels
} Glyph;
```

### Matrix

Hardware abstraction for the LED matrix.

```c
typedef struct {
    uint8_t pin;   // GPIO pin number
    PIO pio;       // PIO instance
    uint8_t sm;    // State machine number
} Matrix;
```

---

## Text Rendering

### matrix_display_char

Display a single character at a specific position.

```c
void matrix_display_char(const char c, int x, int y, const RGB* col);
```

**Parameters:**

- `c` - Character to display (A-Z, 0-9, `!?.-,°`)
- `x` - X coordinate (pixels from left)
- `y` - Y coordinate (pixels from top)
- `col` - Pointer to RGB color

```c
matrix_display_char('A', 0, 1, &RED);
matrix_show(&mtrx);
```

Characters are 3x5 pixels. Call `matrix_show()` to update display.

---

### matrix_display_word

Display a text string with automatic spacing.

```c
void matrix_display_word(const char* word, int x, int y, const RGB* col);
```

**Parameters:**

- `word` - Null-terminated string
- `x` - Starting X coordinate
- `y` - Y coordinate
- `col` - Color for all characters

```c
matrix_display_word("HELLO", 0, 1, &DEFAULT_COLOUR);
matrix_show(&mtrx);
```

---

### matrix_calculate_word_width_with_space

Calculate the total width a word will occupy.

```c
int matrix_calculate_word_width_with_space(const char* word);
```

**Returns:** Width in pixels

```c
int width = matrix_calculate_word_width_with_space("SCORE");
int x_centered = (MATRIX_WIDTH - width) / 2;
```

---

## Icon Rendering

### matrix_display_icon

Display a predefined icon.

```c
void matrix_display_icon(const Glyph* icon, int x, int y, const RGB* col);
```

**Parameters:**

- `icon` - Pointer to icon glyph (from `ICONS_ARR`)
- `x` - X coordinate
- `y` - Y coordinate
- `col` - Color (overwrites icon's default colors if provided)

**Available Icons:**

Access icons via `ICONS_ARR[IconType]`:

```c
typedef enum {
    PHOTON = 0,
    TICKERS = 1,
    TKR_UP = 2,
    TKR_DOWN = 3,
    GAMES = 4,
    TETRIS = 5,
    WEATHER = 6,
    SUNNY = 7,
    SUNNY_CLOUDY = 8,
    CLOUDY = 9,
    RAIN = 10,
    LIGHTNING = 11,
    SNOW = 12,
    MOON = 13,
    WIFI = 14,
    DATA = 15,
} IconType;
```

**Example:**

```c
// Display sunny weather icon
matrix_display_icon(&ICONS_ARR[SUNNY], 10, 1, NULL);
matrix_show(&mtrx);
```

---

### matrix_display_word_icon_pair

Display text with an icon side-by-side.

```c
void matrix_display_word_icon_pair(const char* word, const RGB* word_col,
                                   const Glyph* icon, int offset_x);
```

**Parameters:**

- `word` - Text to display
- `word_col` - Text color
- `icon` - Icon to display after text
- `offset_x` - Horizontal offset for the entire pair

**Example:**

```c
// Display "SUNNY" with sun icon
matrix_display_word_icon_pair("SUNNY", &YELLOW,
                              &ICONS_ARR[SUNNY], 0);
matrix_show(&mtrx);
```

**Layout:**

```
[WORD] [1px gap] [ICON]
```

---

## Pixel Manipulation

### matrix_set_pixel

Set a single pixel to a specific color.

```c
void matrix_set_pixel(Pixel* pxl);
```

**Example:**

```c
Pixel p = {.x = 10, .y = 4, .col = {255, 0, 0}};
matrix_set_pixel(&p);
matrix_show(&mtrx);
```

---

### matrix_clear_pixel

Turn off a pixel at coordinates.

```c
void matrix_clear_pixel(int x, int y);
```

**Example:**

```c
matrix_clear_pixel(15, 3);  // Turn off pixel at (15, 3)
matrix_show(&mtrx);
```

---

### matrix_get_pixel_state

Read the current state of a pixel.

```c
bool matrix_get_pixel_state(int index, Pixel* out_pixel);
```

**Parameters:**

- `index` - Linear index (0-255 for 32x8)
- `out_pixel` - Output pixel data

**Returns:** `true` if pixel is lit, `false` if off

---

## Drawing Primitives

### matrix_draw_horiz_line

Draw a horizontal line.

```c
void matrix_draw_horiz_line(int x, int y, int length, const RGB* col);
```

**Example:**

```c
// Draw 10-pixel green line at row 4
matrix_draw_horiz_line(0, 4, 10, &GREEN);
matrix_show(&mtrx);
```

---

### matrix_clear_horiz_line

Erase a horizontal line.

```c
void matrix_clear_horiz_line(int x, int y, int length);
```

---

### matrix_draw_vert_line

Draw a vertical line.

```c
void matrix_draw_vert_line(int x, int y, int length, const RGB* col);
```

**Example:**

```c
// Draw 5-pixel red vertical line
matrix_draw_vert_line(16, 0, 5, &RED);
matrix_show(&mtrx);
```

---

## Display Control

### matrix_show

Push buffered pixels to the LED matrix.

```c
void matrix_show(Matrix* mtrx);
```

**Important:** All rendering functions (text, icons, pixels) are buffered. Call `matrix_show()` to actually update the display.

**Example:**

```c
matrix_clear(&mtrx);
matrix_display_word("READY", 2, 1, &GREEN);
matrix_show(&mtrx);  // ← Display updates here
```

---

### matrix_clear

Clear all pixels (turn off entire display).

```c
void matrix_clear(Matrix* mtrx);
```

**Example:**

```c
matrix_clear(&mtrx);
matrix_show(&mtrx);  // Display goes black
```

---

### matrix_shift

Shift all pixels by an offset (for scrolling).

```c
void matrix_shift(Matrix* mtrx, int offset_x, int offset_y);
```

**Parameters:**

- `offset_x` - Horizontal shift (negative = left, positive = right)
- `offset_y` - Vertical shift (negative = up, positive = down)

**Example:**

```c
// Scroll left by 1 pixel per frame
for (int i = 0; i < 32; i++) {
    matrix_shift(&mtrx, -1, 0);
    matrix_show(&mtrx);
    sleep_ms(50);
}
```

---

## State Management

### matrix_save_state

Capture current display state for animations.

```c
void matrix_save_state(PixelState* destination);
```

**Example:**

```c
PixelState saved[NUM_PIXELS];
matrix_save_state(saved);

// ... modify display ...

// Restore later
matrix_render_raw(saved, 0, 0);
```

---

### matrix_render_raw

Render a saved pixel state with optional offset.

```c
void matrix_render_raw(const PixelState* source, int offset_x, int offset_y);
```

---

## Graphics Helpers

### graphics_display_start_screen

Show the Photon logo at startup.

```c
void graphics_display_start_screen(Matrix* mtrx);
```

**Example:**

```c
int main() {
    Matrix mtrx = {/* ... */};
    graphics_display_start_screen(&mtrx);
    sleep_ms(2000);  // Show for 2 seconds
}
```

---

## Examples

**Scrolling:**
```c
static int x = MATRIX_WIDTH;
matrix_clear(mtrx);
matrix_display_word("HELLO", x--, 1, &DEFAULT_COLOUR);
matrix_show(mtrx);
```

**Progress bar:**
```c
int length = (MATRIX_WIDTH * percent) / 100;
matrix_draw_horiz_line(0, 7, length, &GREEN);
```

---

**Next:** [Input API](input.md) →
