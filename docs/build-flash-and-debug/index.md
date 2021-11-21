# Build, flash and debug
## Project branches
## Versioning
## Files included in the release notes
## Build the project
## Build the documentation
### Setup
The documentation is written in Makrdown (.md) files and generated using the Sphinx documentation generator.

First, we need to install Sphinx and its dependencies:
 - `myst-parser` : add support for markdown files
 - `sphinx_rtd_theme` : theme from ReadTheDocs
```
pip install sphinx
pip install myst-parser
pip install sphinx_rtd_theme
```

### Build the doc
Run the following command in the folder `docs`
```
sphinx-build -b html ./ ./generated
```

Then display the doc by browsing to `generated/index.html` using your favorite web browser.

## Flash the firmware using OpenOCD and STLinkV2
## Build the project with Docker
## Build the project with VSCode
## Bootloader, OTA and DFU
## Stub using NRF52-DK
## Logging with JLink RTT
## Using files from the releases
