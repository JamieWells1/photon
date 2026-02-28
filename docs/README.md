# Photon Documentation

This directory contains the source for Photon's documentation website, built with [MkDocs Material](https://squidfunk.github.io/mkdocs-material/).

## Local Development

### Install Dependencies

```bash
pip install -r requirements.txt
```

### Preview Documentation

Start the development server with live reload:

```bash
mkdocs serve
```

Then open [http://127.0.0.1:8000](http://127.0.0.1:8000) in your browser.

### Build Documentation

Generate static HTML files:

```bash
mkdocs build
```

Output will be in the `site/` directory.

## Project Structure

```
docs/
├── index.md                      # Home page
├── getting-started/
│   ├── hardware.md              # Hardware setup guide
│   ├── flashing.md              # Firmware flashing
│   └── configuration.md         # Configuration guide
├── progress/
│   └── index.md                 # Build progress log
└── contributing/
    ├── index.md                 # Contributing guidelines
    ├── building-a-mode.md       # Mode development tutorial
    └── api/
        ├── display.md           # Display API reference
        ├── input.md             # Input API reference
        ├── modes.md             # Mode API reference
        └── middleware.md        # Middleware API reference
```

## Contributing

Improvements to documentation are always welcome! See the [Contributing Guide](contributing/index.md).
