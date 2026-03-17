# Photon

Photon is a firmware application for the Raspberry Pi Pico 2 W that transforms a 32x8 WS2812B LED matrix into an interactive display system. The firmware provides a menu-driven interface for displaying real-time weather and cryptocurrency prices, controlled through a rotary encoder and push buttons.

![Gemini_Generated_Image_jdipoejdipoejdip Medium](https://github.com/user-attachments/assets/69394211-9c20-4712-8bbb-81a906645902)
> The intended final design

## Overview

The system connects to WiFi to fetch data from multiple sources and displays dynamic, color-coded information with custom icons. Display weather data from Open-Meteo API or live cryptocurrency prices from Kraken. Navigation through the menu system is handled via a rotary encoder with integrated push button and two additional tactile buttons.

## Features

### Weather Display

Retrieves real-time weather data including current temperature, hourly forecasts, and weather conditions. Temperature values are displayed with dynamic color coding that transitions from blue (cold) through green (moderate) to red (hot). Weather icons automatically adapt based on conditions and time of day. Includes predefined configurations for ten global cities with proper timezone handling.

### Cryptocurrency Ticker

Displays live prices for various cryptocurrencies from Kraken's API. Shows current price and 24-hour percentage change with color-coded up/down indicators. Prices scroll continuously across the display with automatic refresh every 15 minutes.

### Interactive Navigation
A rotary encoder provides smooth navigation through the menu system and hourly weather forecasts. The interface includes quadratic-eased sliding animations between menu states for a polished user experience.

### Power Management
WiFi connectivity is managed intelligently, with the system connecting only to fetch weather data and disconnecting afterward to reduce power consumption. Connection status is displayed on the matrix during network operations.

## Hardware Requirements

- Raspberry Pi Pico 2 W (wireless version required)
- WS2812B LED Matrix (32x8, or equivalent strip arranged in matrix configuration)
- Rotary Encoder (KY-040 or similar with integrated push button)
- Two tactile push buttons for left/right navigation
- 5V power supply (minimum 2A recommended, depending on brightness settings)

## Documentation

Complete documentation is available at [jamiewells1.github.io/photon](https://jamiewells1.github.io/photon/):

- [Hardware Setup](https://jamiewells1.github.io/photon/getting-started/hardware/) - Assembly instructions, pinout diagrams, and wiring guide
- [Flashing Firmware](https://jamiewells1.github.io/photon/getting-started/flashing/) - Step-by-step firmware installation
- [Configuration](https://jamiewells1.github.io/photon/getting-started/configuration/) - WiFi and location setup
- [API Reference](https://jamiewells1.github.io/photon/contributing/) - Display, input, and middleware documentation
- [Building Custom Modes](https://jamiewells1.github.io/photon/contributing/building-a-mode/) - Guide for extending functionality

## Quick Start

1. Assemble the hardware according to the [hardware setup guide](https://jamiewells1.github.io/photon/getting-started/hardware/)
2. Configure your WiFi credentials and location in `src/common/secrets_config.c`
3. Build the firmware with CMake and flash to your Pico 2 W
4. Press any button to start the menu system

Detailed instructions are available in the [getting started documentation](https://jamiewells1.github.io/photon/getting-started/flashing/).

## License

MIT License

## Contributing

Contributions are welcome. Please refer to the [contributing guide](https://jamiewells1.github.io/photon/contributing/) for information on building custom modes and extending the firmware.
