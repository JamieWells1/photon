# Configuration

Configure WiFi credentials, location, and other settings for your Photon display.

## WiFi Configuration

WiFi credentials are compiled into the firmware for security and simplicity.

### Set WiFi Credentials

1. **Create** a `secrets.h` file in the project root:

    ```bash
    touch secrets.h
    ```

2. **Add** your WiFi credentials:

    ```c title="secrets.h"
    #ifndef SECRETS_H
    #define SECRETS_H

    #define WIFI_SSID "YourNetworkName"
    #define WIFI_PASSWORD "YourPassword"

    #endif  // SECRETS_H
    ```

3. **Rebuild** and flash the firmware:

    ```bash
    cmake --build build -- -j4
    picotool load build/photon.uf2 -fx
    ```

!!! warning "Security"
**Never commit `secrets.h` to version control!**

    The file is already in `.gitignore`, but double-check before pushing.

## Location Settings

Set your geographic location for weather data.

### Available Locations

Edit [`src/common/const.h`](https://github.com/JamieWells1/photon/blob/main/src/common/const.h#L44):

```c title="src/common/const.h"
#define LOCATION LOC_TOKYO  // ← Change this
```

**Built-in locations:**

| Location       | Constant             | Coordinates       |
| -------------- | -------------------- | ----------------- |
| London         | `LOC_LONDON`         | 51.66°N, 0.40°E   |
| Miami          | `LOC_MIAMI`          | 25.76°N, 80.19°W  |
| Los Angeles    | `LOC_LOS_ANGELES`    | 34.05°N, 118.24°W |
| Chicago        | `LOC_CHICAGO`        | 41.88°N, 87.63°W  |
| Tokyo          | `LOC_TOKYO`          | 35.68°N, 139.65°E |
| Sydney         | `LOC_SYDNEY`         | 33.87°S, 151.21°E |
| Stockholm      | `LOC_STOCKHOLM`      | 59.33°N, 18.07°E  |
| Rio de Janeiro | `LOC_RIO_DE_JANEIRO` | 22.91°S, 43.17°W  |
| Cape Town      | `LOC_CAPE_TOWN`      | 33.92°S, 18.42°E  |
| Athens         | `LOC_ATHENS`         | 37.98°N, 23.73°E  |

### Add Custom Location

1. **Add latitude constant**:

    ```c title="src/common/const.h"
    #define LAT_PARIS 48.8566
    ```

2. **Add longitude constant**:

    ```c
    #define LON_PARIS 2.3522
    ```

3. **Add to Location enum**:

    ```c
    typedef enum
    {
        LOC_LONDON,
        // ... existing locations
        LOC_PARIS,  // ← Add here
    } Location;
    ```

4. **Set as active location**:

    ```c
    #define LOCATION LOC_PARIS
    ```

## Display Settings

### Brightness

Adjust LED brightness in [`src/common/const.h`](https://github.com/JamieWells1/photon/blob/main/src/common/const.h#L24):

```c title="src/common/const.h"
#define BRIGHTNESS_COEFFICIENT 0.02  // 2% brightness
```

- **Range**: `0.0` (off) to `1.0` (full)
- **Recommended**: `0.01` - `0.05` (1-5%) for indoor use
- **Warning**: High brightness increases power draw significantly

### Update Rate

Change display refresh rate:

```c title="src/common/const.h"
#define DISPLAY_UPDATE_INTERVAL_TICKS_MS 50  // 20 FPS
```

Lower values = smoother animations, higher CPU usage.

## Debug Logging

Control serial output verbosity in [`src/common/const.c`](https://github.com/JamieWells1/photon/blob/main/src/common/const.c):

```c title="src/common/const.c"
const DebugLevel DEBUG_LEVEL = INFO;
```

**Levels:**

| Level    | Value | Description                  |
| -------- | ----- | ---------------------------- |
| `INFO`   | 0     | Important events only        |
| `INPUTS` | 1     | + Button/rotary input events |
| `TRACE`  | 2     | + Detailed execution trace   |

## GPIO Pin Mapping

If your hardware uses different pins, modify [`src/common/const.h`](https://github.com/JamieWells1/photon/blob/main/src/common/const.h#L27):

```c title="src/common/const.h"
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

```c title="src/common/const.h"
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
```

!!! note "Layout Changes Required"
Changing dimensions may require adjustments to display logic in `src/modes/`.

## Weather Settings

Configure weather forecast range:

```c title="src/common/const.h"
#define WEATHER_HOURS 48  // Fetch 48-hour forecast
```

## WiFi Connection Timeout

Adjust max connection attempts:

```c title="src/common/const.h"
#define MAX_WIFI_CONNECTION_ATTEMPTS 100
```

## Apply Changes

After modifying any configuration:

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

---

!!! tip "Configuration Best Practices" - Keep `secrets.h` out of version control - Document custom configurations in code comments - Test after each configuration change
