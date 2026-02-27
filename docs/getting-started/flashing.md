# Flashing Firmware

Learn how to flash the Photon firmware to your Raspberry Pi Pico 2 W.

## Download Firmware

Get the latest firmware release from GitHub:

**[📦 Latest Release](https://github.com/yourusername/photon/releases/latest)**

Download the `photon.uf2` file.

## Method 1: Drag-and-Drop (First Time)

This method is the easiest for first-time setup.

### Steps

1. **Disconnect** the Pico from power

2. **Hold BOOTSEL** button on the Pico

3. **Connect** the Pico to your computer via USB **while holding BOOTSEL**

4. **Release BOOTSEL** when the Pico appears as a USB drive (named `RPI-RP2`)

5. **Drag** the `photon.uf2` file onto the `RPI-RP2` drive

6. The Pico will **automatically reboot** and start running Photon

!!! success "Success!"
    The LED matrix should display the Photon logo, then attempt to connect to WiFi.

## Method 2: picotool (Recommended for Development)

`picotool` allows you to flash without pressing BOOTSEL.

### Install picotool

=== "macOS"
    ```bash
    brew install picotool
    ```

=== "Linux (Ubuntu/Debian)"
    ```bash
    sudo apt install picotool
    ```

=== "Windows"
    Download from [Raspberry Pi GitHub](https://github.com/raspberrypi/picotool/releases)

### Flash with picotool

```bash
# Flash and force reboot
picotool load photon.uf2 -fx
```

!!! tip "Development Workflow"
    For rapid development, use the all-in-one command from `INSTRUCTIONS.md`:

    ```bash
    cmake --build build -- -j4 && picotool load build/photon.uf2 -fx && screen /dev/tty.usbmodem101 115200
    ```

    This builds, flashes, and opens serial monitor in one command!

## Method 3: Build from Source

If you want to modify the firmware, build it yourself.

### Prerequisites

=== "macOS"
    ```bash
    brew install cmake gcc-arm-embedded
    ```

=== "Linux"
    ```bash
    sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
    ```

### Install Pico SDK

```bash
# Clone Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git --branch 2.2.0 --depth 1
cd pico-sdk
git submodule update --init
cd ..

# Set environment variable
export PICO_SDK_PATH=$(pwd)/pico-sdk
```

### Build Photon

```bash
# Configure CMake
cmake -B build -DPICO_BOARD=pico2_w

# Build firmware
cmake --build build -- -j4
```

The compiled firmware will be at `build/photon.uf2`.

## Verify Installation

After flashing, check the serial output to verify:

### Monitor Serial Output

=== "macOS/Linux"
    ```bash
    screen /dev/tty.usbmodem101 115200
    ```

    Press `Ctrl+A` then `K` then `Y` to exit screen.

=== "Windows"
    Use [PuTTY](https://www.putty.org/) or Arduino Serial Monitor:

    - Port: `COMx` (check Device Manager)
    - Baud: `115200`

### Expected Output

```
[PHOTON] Starting...
[WIFI] Connecting to WiFi...
[WIFI] Connected! IP: 192.168.1.100
[MENU] Main menu started
```

## Troubleshooting

### Pico not appearing as USB drive

- Ensure you're holding BOOTSEL **before** connecting USB
- Try a different USB cable (some are charge-only)
- Try a different USB port

### `picotool` can't find device

```bash
# Check if Pico is detected
picotool info

# May need sudo on Linux
sudo picotool info
```

### Build errors

- Verify `PICO_SDK_PATH` is set correctly
- Ensure you're using Pico SDK 2.2.0
- Check that all submodules are initialized

### No serial output

- Enable USB serial in `CMakeLists.txt` (already enabled by default):
  ```cmake
  pico_enable_stdio_usb(photon 1)
  ```

## Next Steps

- [Configure WiFi & Location](configuration.md) →
- [Understand the API](../contributing/api/display.md) →

---

!!! question "Need Help?"
    [Open an issue on GitHub](https://github.com/yourusername/photon/issues)
