# Contributing to Photon

Thank you for your interest in contributing to Photon! This guide will help you understand the codebase and start developing new features.

## Getting Started

Before diving into code, make sure you have:

1. ✅ Completed the [Hardware Setup](../getting-started/hardware.md)
2. ✅ Built the project from source (see [Flashing Guide](../getting-started/flashing.md))
3. ✅ Read through the codebase structure below

## Project Structure

```
src/
├── common/       # Constants and utilities
├── display/      # Matrix control, text, icons
├── input/        # Buttons and rotary encoder
├── middleware/   # WiFi and HTTP
└── modes/        # Display modes (ticker, weather, games)
```

## How to Contribute

1. Fork and create a branch
2. Make your changes
3. Test on hardware
4. Submit a pull request

## What to Contribute

### 🎮 New Display Modes

The most common contribution! Add new modes like:

- Games (Snake, Pong, Conway's Game of Life)
- Utilities (Pomodoro timer, countdown)
- Data displays (Crypto prices, sports scores)
- Visualizations (Music reactive, animations)

**Start here**: [Building a Mode Tutorial](building-a-mode.md) →

### 🎨 Graphics & Fonts

- New icon designs
- Extended character sets (lowercase, symbols)
- Animation helpers

### 🔧 Core Improvements

- Performance optimizations
- Bug fixes
- API enhancements
- Documentation improvements

### 🌐 Network Features

- New API integrations
- MQTT support
- WebSocket connections

## API Reference

Understand the public API before building features:

- **[Display API](api/display.md)** - Matrix control, text, icons
- **[Input API](api/input.md)** - Buttons and rotary encoder
- **[Mode API](api/modes.md)** - Display mode structure
- **[Middleware API](api/middleware.md)** - WiFi and HTTP

## Code Style

- Functions: `module_action_object()` (e.g., `matrix_display_word()`)
- Structs: `PascalCase`
- Constants: `UPPER_SNAKE_CASE`
- Static vars: `g_` prefix
- Comment why, not what

## Testing

Before submitting:

- Builds without warnings
- Test on actual hardware
- Check input responds correctly
- Verify display updates smoothly

## Get Help

- **Questions?** [Open a discussion](https://github.com/JamieWells1/photon/discussions)
- **Bug?** [Report an issue](https://github.com/JamieWells1/photon/issues)
- **Feature idea?** [Open a feature request](https://github.com/JamieWells1/photon/issues/new)

---

**Ready to build?** Start with the [Building a Mode Tutorial](building-a-mode.md) →
