# Delta

Delta is a firmware application for the Raspberry Pi Pico 2 W that transforms a 32x8 WS2812B LED matrix into a real-time financial and weather display. It shows live stock prices, cryptocurrency tickers, FX currency pairs, portfolio tracking, and weather data, all controlled through a rotary encoder and push buttons.

![Gemini_Generated_Image_tscd4utscd4utscd Medium](https://github.com/user-attachments/assets/11c758ab-5cbd-4271-a3f6-5426cf373bc0)
> The intended final design

## Overview

The system connects to WiFi and fetches data from multiple sources via an HTTPS proxy server. A web-based configuration portal at `delta.local` allows users to manage watchlists, upload portfolios, and adjust settings from any device on the same network. Navigation through the menu system is handled via a rotary encoder with integrated push button and two additional tactile buttons.

## Features

### Market Data

- **Stocks** — Live prices for custom watchlists with daily percentage change. Supports US and LSE tickers with automatic `.L` suffix translation.
- **Cryptocurrency** — Live prices from Kraken's API with 24-hour percentage change.
- **Currency Pairs** — Real-time FX rates for up to 10 configurable pairs (e.g. GBPUSD, GBPEUR). Includes sensible defaults out of the box.
- **Portfolio** — Upload a CSV export (e.g. from Trading 212) and track total P&L, percentage change, biggest winner/loser, and total value. Supports both daily and all-time modes. Auto-scroll displays all metrics as a continuous ticker tape, or use manual mode to browse with the encoder.

All market data scrolls continuously across the display with automatic refresh every 10 minutes.

### Weather Display

Retrieves real-time weather data including current temperature, hourly forecasts, and weather conditions. Temperature values are displayed with dynamic color coding that transitions from blue (cold) through green (moderate) to red (hot). Weather icons automatically adapt based on conditions and time of day. Includes predefined configurations for 32 global cities with proper timezone handling.

### Web Configuration Portal

Access `delta.local` from any browser on the same network to:
- Set location, currency, and brightness
- Manage stock and crypto watchlists
- Configure FX currency pairs
- Upload portfolio CSV files and choose daily/all-time P&L mode
- Set auto or manual scroll for portfolio display
- Update firmware OTA

### Interactive Navigation

A rotary encoder provides smooth navigation through the menu system and hourly weather forecasts. The interface includes quadratic-eased sliding animations between menu states for a polished user experience.

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
2. Build the firmware with CMake and flash to your Pico 2 W
3. On first boot, connect to the `Delta-Setup` WiFi AP and enter your network credentials
4. Once connected, navigate to `delta.local` to configure your watchlists and settings

Detailed instructions are available in the [getting started documentation](https://jamiewells1.github.io/photon/getting-started/flashing/).

## License

MIT License

## Contributing

Contributions are welcome. Please refer to the [contributing guide](https://jamiewells1.github.io/photon/contributing/) for information on building custom modes and extending the firmware.
