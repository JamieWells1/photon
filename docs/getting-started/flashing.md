# Flashing Firmware

Learn how to flash the Photon firmware to your Raspberry Pi Pico 2 W.

## Download Firmware

Get the latest firmware release from GitHub:

**[📦 Latest Release](https://github.com/yourusername/photon/releases/latest)**

Download the `photon.uf2` file.

## Flash via picotool

`picotool` allows you to flash without pressing BOOTSEL.

### Install picotool

=== "macOS"
`bash
    brew install picotool
    `

=== "Linux (Ubuntu/Debian)"
`bash
    sudo apt install picotool
    `

=== "Windows"
Download from [Raspberry Pi GitHub](https://github.com/raspberrypi/picotool/releases)

### Flash with picotool

```bash
# Flash and force reboot
picotool load photon.uf2 -fx
```

For rapid development, use the all-in-one command from `INSTRUCTIONS.md`:

    ```bash
    cmake --build build -- -j4 && picotool load build/photon.uf2 -fx && screen /dev/tty.usbmodem1101 115200
    ```

    This builds, flashes, and opens serial monitor in one command!

    !!! circle-info "The path to your USB modem may not be exactly `/dev/tty.usbmodem1101`, and you may have to see what it actually is."

## Method 3: Build from Source

If you want to modify the firmware, build it yourself.

### Prerequisites

=== "macOS"
`bash
    brew install cmake gcc-arm-embedded
    `

=== "Linux"
`bash
    sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
    `

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
