# Input API

The Input API handles button presses and rotary encoder events with built-in debouncing.

## Core Types

### Button

Represents a single push button with state tracking.

```c
typedef struct {
    uint8_t pin;           // GPIO pin number
    bool current_state;    // Current reading
    bool last_state;       // Previous reading
} Button;
```

### Rotator

Represents a rotary encoder with quadrature decoding.

```c
typedef struct {
    uint8_t pin_clk;           // Clock pin (A phase)
    uint8_t pin_dt;            // Data pin (B phase)
    uint8_t pin_sw;            // Switch/button pin
    bool current_clk_state;    // Current CLK state
    bool current_dt_state;     // Current DT state
    bool current_sw_state;     // Current switch state
    bool last_clk_state;       // Previous CLK state
    bool last_sw_state;        // Previous switch state
    uint8_t state;             // Quadrature state (0-3)
    int8_t direction;          // Last direction: 1=CW, -1=CCW, 0=none
    uint32_t last_change_time; // For debouncing
} Rotator;
```

---

## Update Functions

### input_update

Update all input states (buttons + rotary encoder).

```c
void input_update(Button* btns, Rotator* rtr);
```

**Parameters:**

- `btns` - Array of Button structs (size 2 for left/right buttons)
- `rtr` - Pointer to Rotator struct

Call once per frame in main loop.

---

### input_btn_update

Update a single button's state.

```c
void input_btn_update(Button* btn);
```

---

## Button Events

### input_btn_pressed

Check if button was just pressed (rising edge).

```c
bool input_btn_pressed(Button* btn);
```

**Returns:** `true` on first frame of press (edge-triggered)

```c
if (input_btn_pressed(&btns[0])) {
    navigate_left();
}
```

---

### input_btn_released

Check if button was just released (falling edge).

```c
bool input_btn_released(Button* btn);
```

**Returns:** `true` on first frame after release

---

### input_any_btn_pressed

Check if any button or rotary switch was pressed.

```c
bool input_any_btn_pressed(Button* btns, Rotator* rtr);
```

**Parameters:**

- `btns` - Array of 2 buttons (left, right)
- `rtr` - Rotary encoder

**Returns:** `true` if left, right, or encoder button was pressed

**Example:**

```c
// Wake from screensaver on any input
if (input_any_btn_pressed(btns, &rtr)) {
    screensaver_active = false;
}
```

---

## Rotary Encoder Events

### input_rtr_cw

Check if rotary encoder was rotated clockwise.

```c
bool input_rtr_cw(Rotator* rtr);
```

**Returns:** `true` if rotated clockwise since last check

**Example:**

```c
if (input_rtr_cw(&rtr)) {
    current_menu_index++;
    if (current_menu_index >= MENU_COUNT) {
        current_menu_index = 0;
    }
}
```

**Note:** This is a **consuming read** - the event is cleared after returning `true`.

---

### input_rtr_anti_cw

Check if rotary encoder was rotated counter-clockwise.

```c
bool input_rtr_anti_cw(Rotator* rtr);
```

**Returns:** `true` if rotated counter-clockwise

**Example:**

```c
if (input_rtr_anti_cw(&rtr)) {
    current_menu_index--;
    if (current_menu_index < 0) {
        current_menu_index = MENU_COUNT - 1;
    }
}
```

---

### input_rtr_pressed

Check if rotary encoder button was pressed.

```c
bool input_rtr_pressed(Rotator* rtr);
```

**Returns:** `true` on first frame of press

**Example:**

```c
if (input_rtr_pressed(&rtr)) {
    printf("Encoder button pressed - selecting menu\n");
    select_current_menu();
}
```

---

### input_rtr_released

Check if rotary encoder button was released.

```c
bool input_rtr_released(Rotator* rtr);
```

**Returns:** `true` on first frame after release

---

## Example

```c
void handle_input(Button* btns, Rotator* rtr) {
    if (input_rtr_cw(rtr)) {
        next_item();
    }
    if (input_rtr_anti_cw(rtr)) {
        prev_item();
    }
    if (input_rtr_pressed(rtr)) {
        select_item();
    }
    if (input_btn_pressed(&btns[0])) {
        do_action();
    }
}
```

---

**Next:** [Mode API](modes.md) →
