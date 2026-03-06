# Configuration

Configure WiFi credentials, location, and other settings for your Photon display.

## Initial WiFi Setup

On first boot (or after a factory reset), Photon enters **setup mode** automatically:

1. The device creates a WiFi access point called **"Photon-Setup"**
2. The matrix displays **"ENTER WIFI"** with a WiFi icon
3. Connect to the "Photon-Setup" network from your phone or computer
4. A captive portal opens automatically (or navigate to any URL)
5. Enter your WiFi network name (SSID) and password
6. Press **Connect to WiFi**
7. The device saves credentials to flash, displays **"SAVED"**, and reboots

After reboot, Photon connects to your WiFi network and enters the main menu.

## Device Settings

Once connected to WiFi, use the **CONFIG** mode to change settings:

1. Use the rotary encoder to scroll to **CONFIG** in the menu
2. Press the button to enter CONFIG mode
3. The matrix scrolls `http://photon.local`
4. Open that URL in a browser on the same WiFi network
5. Adjust settings:

| Setting      | Options                                      | Default  |
| ------------ | -------------------------------------------- | -------- |
| **Brightness** | 1-25%                                      | 8%       |
| **Location**   | 18 cities (London, New York, Tokyo, etc.)  | London   |
| **Currency**   | 12 currencies (GBP, USD, EUR, JPY, etc.)  | GBP      |

6. Press **Save & Reboot** -- settings are saved to flash and the device reboots

All settings persist across power cycles.

## Factory Reset

To erase all saved configuration and return to setup mode:

1. Use the rotary encoder to scroll to **RESET** in the menu
2. Press the button to enter RESET mode
3. Select **YES** or **NO** using the rotary encoder
4. Press the button to confirm
5. If YES: flash is erased, device reboots into setup mode

## Display Settings

### Brightness

Brightness is set via the CONFIG web interface (1-25%). Values above 10% display a warning recommending an external power supply.

### Update Rate

Change display refresh rate in `src/common/const.h`:

```c
#define DISPLAY_UPDATE_INTERVAL_TICKS_MS 50  // 20 FPS
```

Lower values = smoother animations, higher CPU usage.

## Debug Logging

Control serial output verbosity in `src/common/const.c`:

```c
const LogLevel LOG_LEVEL = INFO;
```

**Levels:**

| Level   | Value | Description                                    |
| ------- | ----- | ---------------------------------------------- |
| `ERROR` | 0     | Errors only                                    |
| `WARN`  | 1     | Warnings + errors                              |
| `INFO`  | 2     | General info + above (default)                 |
| `DEBUG` | 3     | Detailed debugging info + above                |
| `TRACE` | 4     | Verbose execution trace + above                |

## GPIO Pin Mapping

If your hardware uses different pins, modify `src/common/const.h`:

```c
// Buttons
#define BUTTON_LEFT_PIN 10   // GP10
#define BUTTON_RIGHT_PIN 5   // GP5

// Rotary Encoder
#define ROTATOR_CLK_PIN 2    // GP2
#define ROTATOR_DT_PIN 3     // GP3
#define ROTATOR_SW_PIN 4     // GP4

// LED Matrix
#define MATRIX_DIN_PIN 21    // GP21
```

!!! danger "Pin Conflicts"
    Avoid pins used by WiFi (GP23-25, GP29) and reserved system pins.

## Matrix Dimensions

For different LED matrix sizes, update:

```c
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
```

!!! note "Layout Changes Required"
    Changing dimensions may require adjustments to display logic in `src/modes/`.

## Weather Settings

Configure weather forecast range:

```c
#define WEATHER_HOURS 48  // Fetch 48-hour forecast
```

## WiFi Connection Timeout

Adjust max connection attempts:

```c
#define MAX_WIFI_CONNECTION_ATTEMPTS 100
```

## Apply Changes

After modifying any compile-time configuration:

1. **Save** the files

2. **Rebuild**:

   ```bash
   cmake --build build -- -j4
   ```

3. **Flash**:

   ```bash
   picotool load build/photon.uf2 -fx
   ```

## Next Steps

- [View Build Progress](../progress/index.md) →
- [Contribute to Photon](../contributing/index.md) →
