# Photon

**A Raspberry Pi Pico W-powered LED matrix display and game engine**

![Photon Logo](https://via.placeholder.com/600x200/1e3a8a/ffffff?text=PHOTON)

## Overview

Photon is an embedded firmware project that transforms a Raspberry Pi Pico 2 W into a versatile LED matrix display system with WiFi connectivity. It features a menu-driven interface controlled by physical buttons and a rotary encoder, supporting multiple display modes including stock tickers, weather information, and games.

## Features

### 🎮 Multiple Display Modes

- **Tickers**: Real-time stock and commodity prices (BTC, Gold, Silver)
- **Games**: Play Tetris directly on the LED matrix
- **Weather**: Current temperature and hourly forecasts for multiple locations

### 🎨 Custom Graphics Engine

- 32x8 WS2812 LED matrix support
- Custom bitmap font rendering (A-Z, 0-9, punctuation)
- Built-in icon library (weather, games, status indicators)
- RGB color support with brightness control

### 🌐 WiFi Connectivity

- Fetch live data from external APIs
- Asynchronous connection management
- HTTP request handling

### 🕹️ Interactive Controls

- Two programmable buttons (left/right navigation)
- Rotary encoder with push button
- Real-time input debouncing

## Hardware Requirements

- Raspberry Pi Pico 2 W
- 32x8 WS2812/WS2812B LED matrix
- Rotary encoder (with push button)
- 2x tactile push buttons
- Power supply (5V, depends on LED count)

## Quick Start

1. **[Set up the hardware](getting-started/hardware.md)** - Wire the components
2. **[Flash the firmware](getting-started/flashing.md)** - Upload the `.uf2` file
3. **[Configure WiFi](getting-started/configuration.md)** - Set your credentials

## Project Structure

```
photon/
├── src/
│   ├── common/         # Shared constants and utilities
│   ├── display/        # Matrix control and graphics engine
│   ├── input/          # Button and rotary encoder handling
│   ├── middleware/     # WiFi and HTTP networking
│   └── modes/          # Display modes (ticker, weather, games)
├── docs/               # This documentation
└── .github/            # CI/CD workflows
```

## Contributing

Photon welcomes contributions! Check out the [Contributing Guide](contributing/index.md) to learn how to:

- Understand the [API Reference](contributing/api/display.md)
- [Build your own display mode](contributing/building-a-mode.md)
- Submit pull requests

## License

[Add your license here]

---

**Ready to get started?** Head to [Hardware Setup](getting-started/hardware.md) →
