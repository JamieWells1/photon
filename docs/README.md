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

## Deployment

Documentation is automatically built and deployed to GitHub Pages when changes are pushed to the `main` branch via the [`.github/workflows/docs.yml`](../.github/workflows/docs.yml) workflow.

### Manual Deployment

```bash
mkdocs gh-deploy
```

## Adding Content

### New Page

1. Create a Markdown file in the appropriate directory
2. Add it to the `nav` section in `mkdocs.yml`
3. Preview with `mkdocs serve`

### Code Blocks

Use fenced code blocks with syntax highlighting:

\`\`\`c
void example() {
    printf("Hello, Photon!\\n");
}
\`\`\`

### Admonitions

Create callout boxes:

\`\`\`
!!! note "Title"
    This is a note

!!! warning
    This is a warning

!!! tip
    This is a tip
\`\`\`

## Contributing

Improvements to documentation are always welcome! See the [Contributing Guide](contributing/index.md).
