# Hardware Setup

This guide will walk you through assembling the Photon LED matrix display.

## Bill of Materials

| Component                          | Quantity | Notes                                         |
| ---------------------------------- | -------- | --------------------------------------------- |
| Raspberry Pi Pico 2 W              | 1        | Must be the wireless version                  |
| WS2812B LED Matrix (32x8)          | 1        | Or equivalent WS2812 strip arranged in matrix |
| Rotary Encoder (KY-040 or similar) | 1        | With integrated push button                   |
| Tactile Push Buttons               | 2        | For left/right navigation                     |
| 5V Power Supply                    | 1        | Min 2A recommended (depends on brightness)    |
| Jumper Wires                       | ~15      | Male-to-female recommended                    |
| Breadboard (optional)              | 1        | For prototyping                               |

## Pinout Diagram

### Raspberry Pi Pico 2 W

```
┌─────────────────────────────┐
│                             │
│   ●  GP0           VBUS  ●  │ ← 5V Power (from USB or external)
│   ●  GP1            VSYS ●  │
│   ●  GND             GND ●  │
│   ●  GP2 (ROT_CLK)  3V3  ●  │
│   ●  GP3 (ROT_DT)         ●  │
│   ●  GP4 (ROT_SW)         ●  │
│   ●  GP5 (BTN_RIGHT)      ●  │
│   ●  GND                  ●  │
│        ...                   │
│   ●  GP10 (BTN_LEFT)      ●  │
│        ...                   │
│   ●  GP21 (MATRIX_DIN)    ●  │ → LED Matrix Data In
│                             │
└─────────────────────────────┘
```

## Wiring Guide

### LED Matrix (WS2812B)

The LED matrix has three connections:

| Matrix Pin     | Connect To   | Notes         |
| -------------- | ------------ | ------------- |
| **VCC** (5V)   | VBUS on Pico | Power rail    |
| **GND**        | GND on Pico  | Common ground |
| **DIN** (Data) | GP21         | Signal line   |

!!! warning "Power Considerations"
At full brightness, a 32x8 (256 LED) matrix can draw up to 15A (5V). For safety and longevity:

    - Use external power supply (not USB) for production
    - Set `BRIGHTNESS_COEFFICIENT` low (default: 0.02 = 2%)
    - Never exceed your power supply's rating

### Rotary Encoder

| Encoder Pin | Connect To | Function     |
| ----------- | ---------- | ------------ |
| **CLK**     | GP2        | Clock signal |
| **DT**      | GP3        | Data signal  |
| **SW**      | GP4        | Push button  |
| **+**       | 3V3        | Power        |
| **GND**     | GND        | Ground       |

### Push Buttons

| Button       | Connect To | Pull-up/down     |
| ------------ | ---------- | ---------------- |
| Left Button  | GP10 → GND | Internal pull-up |
| Right Button | GP5 → GND  | Internal pull-up |

!!! tip "Button Wiring"
Connect one side of each button to the GPIO pin and the other side to GND. The firmware enables internal pull-up resistors, so no external resistors are needed.

## Assembly Steps

1. **Safety First**: Disconnect all power before wiring

2. **Connect the LED Matrix**:
   - Wire VCC to VBUS (5V)
   - Wire GND to GND
   - Wire DIN to GP21

3. **Connect the Rotary Encoder**:
   - Wire CLK → GP2
   - Wire DT → GP3
   - Wire SW → GP4
   - Wire + → 3V3
   - Wire GND → GND

4. **Connect the Buttons**:
   - Left button: GP10 ↔ GND
   - Right button: GP5 ↔ GND

5. **Double-check connections** against the pinout diagram

6. **Power on** and verify the startup sequence

## Verify Hardware

After powering on, you should see:

1. **Startup animation**: The Photon logo displays briefly
2. **WiFi connection**: Animated connecting screen
3. **Main menu**: Menu icons appear

If nothing displays:

- Check LED matrix data pin (GP21)
- Verify power connections (VBUS, GND)
- Ensure correct voltage (WS2812 needs 5V for data logic)

## Next Steps

- [Flash the Firmware](flashing.md) →
- [Configure WiFi & Location](configuration.md) →

---

!!! example "Example Build"
Add photos of your completed build here!

    ```
    [Photo 1: Breadboard prototype]
    [Photo 2: Close-up of wiring]
    [Photo 3: Finished enclosure]
    ```
