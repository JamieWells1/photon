# Configuration

Configure WiFi credentials, location, and other settings for your Photon display.

## WiFi Configuration

Photon uses a captive portal for easy WiFi setup on first boot. WiFi credentials are stored persistently in flash memory.

### First-Time Setup

On first boot (or after a factory reset), Photon will:

1. **Create an access point** named `Photon-Setup`
2. **Display "ENTER WIFI"** alternating with "WIFI" on the LED matrix
3. **Wait for you to configure WiFi** credentials

**To configure WiFi:**

1. **Connect** to the `Photon-Setup` WiFi network from your phone or computer
2. **Open a browser** - you should be automatically redirected to the setup page
   - If not redirected, navigate to `http://192.168.4.1`
3. **Enter your WiFi credentials** in the web form
4. **Submit** - the device will save credentials and reboot
5. **Matrix displays "SAVED"** and the device reboots automatically

### Reset WiFi Credentials

To reconfigure WiFi or reset credentials:

1. **Navigate to the Reset menu** on your device (4th menu option)
2. **Select "YES"** using the rotary encoder
3. **Confirm** by pressing any button
4. Device will **erase credentials** and **reboot** into setup mode

Alternatively, you can access the reset menu by pressing all buttons simultaneously while in the main menu.

### Manual Configuration (Advanced)

For development builds, you can optionally provide default credentials at compile-time:

1. **Create** a `secrets_config.h` file in the project root:

   ```bash
   touch secrets_config.h
   ```

2. **Add** your WiFi credentials:

   ```c title="secrets_config.h"
   #ifndef SECRETS_CONFIG_H
   #define SECRETS_CONFIG_H

   // Optional: Define default WiFi credentials for development
   // These will be used if no config exists in flash
   // #define DEFAULT_WIFI_SSID "YourNetworkName"
   // #define DEFAULT_WIFI_PASSWORD "YourPassword"

   #endif  // SECRETS_CONFIG_H
   ```

3. **Rebuild** and flash the firmware:

   ```bash
   cmake --build build -- -j4
   picotool load build/photon.uf2 -fx
   ```

!!! warning "Security"
**Never commit `secrets_config.h` with real credentials to version control!**

    The file is already in `.gitignore`, but double-check before pushing.

## Location Settings

Set your geographic location for weather data.

### Available Locations

Edit [`src/config.h`](https://github.com/JamieWells1/photon/blob/main/src/config.h):

```c title="src/config.h"
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

Adjust LED brightness in [`src/config.h`](https://github.com/JamieWells1/photon/blob/main/src/config.h):

```c title="src/config.h"
#define BRIGHTNESS 0.02  // 2% brightness
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
const LogLevel LOG_LEVEL = INFO;
```

**Levels:**

| Level   | Value | Description                                    |
| ------- | ----- | ---------------------------------------------- |
| `ERROR` | 0     | Errors only (always log critical failures)     |
| `WARN`  | 1     | Warnings + errors                              |
| `INFO`  | 2     | General info + above (default)                 |
| `DEBUG` | 3     | Detailed debugging info + above                |
| `TRACE` | 4     | Verbose execution trace + above (most verbose) |

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

## Currency Settings

Set your preferred currency for cryptocurrency price display.

Edit [`src/config.h`](https://github.com/JamieWells1/photon/blob/main/src/config.h):

```c title="src/config.h"
#define CURRENCY CCY_USD  // ← Change this
```

**Available currencies:**

- `CCY_USD` - US Dollar
- `CCY_EUR` - Euro
- `CCY_GBP` - British Pound
- `CCY_JPY` - Japanese Yen

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
