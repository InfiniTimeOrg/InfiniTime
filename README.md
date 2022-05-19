# [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime)

[![Build PineTime Firmware](https://github.com/InfiniTimeOrg/InfiniTime/workflows/Build%20PineTime%20Firmware/badge.svg?branch=master)](https://github.com/InfiniTimeOrg/InfiniTime/actions)

![InfiniTime logo](images/infinitime-logo-small.jpg "InfiniTime Logo")

Fast open-source firmware for the [PineTime smartwatch](https://www.pine64.org/pinetime/) with many features, written in modern C++.

## New to InfiniTime?

 - [Getting started with InfiniTime](doc/gettingStarted/gettingStarted-1.0.md)
 - [Updating the software](doc/gettingStarted/updating-software.md)
 - [About the firmware and bootloader](doc/gettingStarted/about-software.md)
### Companion apps
 - [Gadgetbridge](https://gadgetbridge.org/) (Android)
 - [AmazFish](https://openrepos.net/content/piggz/amazfish/) (SailfishOS)
 - [Siglo](https://github.com/alexr4535/siglo) (Linux)
 - [InfiniLink](https://github.com/InfiniTimeOrg/InfiniLink) **[Experimental]** **[Unmaintained, looking for developers/maintainers]** (iOS)
 - [ITD](https://gitea.arsenm.dev/Arsen6331/itd) (Linux)

## Development

 - [Rough structure of the code](doc/code/Intro.md)
 - [How to implement an application](doc/code/Apps.md)
 - [Generate the fonts and symbols](src/displayapp/fonts/README.md)
 - [Creating a stopwatch in Pinetime(article)](https://pankajraghav.com/2021/04/03/PINETIME-STOPCLOCK.html)
 - [Tips on designing an app UI](doc/ui_guidelines.md)

### InfiniSim Simulator
Use the [InfiniSim Simulator](https://github.com/InfiniTimeOrg/InfiniSim) to experience the `InfiniTime` user interface directly on your PC, to shorten the time until you get your hands on a real [PineTime smartwatch](https://www.pine64.org/pinetime/).
Or use it to develop new Watchfaces, new Screens, or quickly iterate on the user interface.

### Contributing
 - [How to contribute?](/doc/contribute.md)
 - [Coding conventions](/doc/coding-convention.md)

### Build, flash and debug

 - [Project branches](doc/branches.md)
 - [Versioning](doc/versioning.md)
 - [Files included in the release notes](doc/filesInReleaseNotes.md)
 - [Build the project](doc/buildAndProgram.md)
 - [Flash the firmware using OpenOCD and STLinkV2](doc/openOCD.md)
 - [Flash the firmware using SWD interface](doc/SWD.md)
 - [Build the project with Docker](doc/buildWithDocker.md)
 - [Build the project with VSCode](doc/buildWithVScode.md)
 - [Bootloader, OTA and DFU](./bootloader/README.md)
 - [Stub using NRF52-DK](./doc/PinetimeStubWithNrf52DK.md)

### API

 - [BLE implementation and API](./doc/ble.md)

### Architecture and technical topics

 - [Memory analysis](./doc/MemoryAnalysis.md)

## Licenses

This project is released under the GNU General Public License version 3 or, at your option, any later version.

It integrates the following projects:
 - RTOS : **[FreeRTOS](https://freertos.org)** under the MIT license
 - UI : **[LittleVGL/LVGL](https://lvgl.io/)** under the MIT license
 - BLE stack : **[NimBLE](https://github.com/apache/mynewt-nimble)** under the Apache 2.0 license
 - Font : **[Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)** under the Apache 2.0 license

## Credits

I’m not working alone on this project. First, many people create PR for this projects. Then, there is the whole #pinetime community : a lot of people all around the world who are hacking, searching, experimenting and programming the Pinetime. We exchange our ideas, experiments and code in the chat rooms and forums.

Here are some people I would like to highlight:

 - [Atc1441](https://github.com/atc1441/) : He works on an Arduino based firmware for the Pinetime and many other smartwatches based on similar hardware. He was of great help when I was implementing support for the BMA421 motion sensor and I²C driver.
 - [Koen](https://github.com/bosmoment) : He’s working on a firmware based on RiotOS. He integrated similar libs as me : NimBLE, LittleVGL,… His help was invaluable too!
 - [Lup Yuen Lee](https://github.com/lupyuen) : He is everywhere: he works on a Rust firmware, builds a MCUBoot based bootloader for the Pinetime, designs a Flutter based companion app for smartphones and writes a lot of articles about the Pinetime!

*If you feel like you should appear on this list, just get in touch with me or submit a PR :)*
