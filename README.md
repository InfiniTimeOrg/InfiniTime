<div align="center">

![Header Image](doc/logo/watchface_collage.png)

<br>

[![GitHub tag](https://img.shields.io/github/tag/InfiniTimeOrg/InfiniTime?include_prereleases=&sort=semver&color=blue)](https://github.com/InfiniTimeOrg/InfiniTime/releases)
[![GitHub License](https://img.shields.io/github/license/InfiniTimeOrg/InfiniTime)](https://github.com/InfiniTimeOrg/InfiniLink/blob/main/LICENSE)
[![Issues - InfiniTime](https://img.shields.io/github/issues/InfiniTimeOrg/InfiniTime)](https://github.com/InfiniTimeOrg/InfiniTime/issues)
[![Pull Requests - InfiniTime](https://img.shields.io/github/issues-pr/InfiniTimeOrg/InfiniTime)](https://github.com/InfiniTimeOrg/InfiniTime/pulls)
[![Downloads - InfiniTime](https://img.shields.io/github/downloads/InfiniTimeOrg/InfiniTime/total)](https://github.com/InfiniTimeOrg/InfiniTime)
[![Stars - InfiniTime](https://img.shields.io/github/stars/InfiniTimeOrg/InfiniTime?style=social)](https://github.com/InfiniTimeOrg/InfiniTime/stargazers)
[![Forks - InfiniTime](https://img.shields.io/github/forks/InfiniTimeOrg/InfiniTime?style=social)](https://github.com/InfiniTimeOrg/InfiniTime/network/members)

# InfiniTime

*Fast open-source firmware for the [PineTime smartwatch](https://pine64.org/devices/pinetime/) with many features, written in modern C++.*

<br>

</div>

## New to InfiniTime?

- [Getting started with InfiniTime](doc/gettingStarted/gettingStarted-1.0.md)
- [Updating the software](doc/gettingStarted/updating-software.md)
- [About the firmware and bootloader](doc/gettingStarted/about-software.md)
- [Available apps](doc/gettingStarted/Applications.md)
- [Available watch faces](/doc/gettingStarted/Watchfaces.md)
- [PineTimeStyle Watch face](https://pine64.org/documentation/PineTime/Watchfaces/PineTimeStyle)
  - [Weather integration](https://pine64.org/documentation/PineTime/Software/InfiniTime_weather/)

### Companion apps

- [Gadgetbridge](https://gadgetbridge.org/) (Android)
- [Amazfish](https://github.com/piggz/harbour-amazfish/) ([SailfishOS](https://sailfishos-chum.github.io/apps/harbour-amazfish/), [Ubuntu Touch](https://open-store.io/app/uk.co.piggz.amazfish), [Flatpak](https://flathub.org/apps/uk.co.piggz.amazfish))
- [Siglo](https://github.com/alexr4535/siglo) (Linux)
- [InfiniLink](https://github.com/InfiniTimeOrg/InfiniLink) (iOS)
- [ITD](https://gitea.elara.ws/Elara6331/itd) (Linux)
- [WatchMate](https://github.com/azymohliad/watchmate) (Linux)
- [InfiniTimeExplorer](https://infinitimeexplorer.netlify.app) (Web)

<br>

> *InfiniTimeExplorer is only compatible with web browsers that support Web BLE. Current fully supported browsers include Chrome and Microsoft Edge.* 
>
> *We removed mentions to NRFConnect as this app is closed source and recent versions do not work anymore with InfiniTime (the last version known to work is 4.24.3). If you used NRFConnect in the past, we recommend you switch to [Gadgetbridge](https://gadgetbridge.org/).* 

## Development

- [InfiniTime Vision](doc/InfiniTimeVision.md)
- [Rough structure of the code](doc/code/Intro.md)
- [How to implement an application](doc/code/Apps.md)
- [Generate the fonts and symbols](src/displayapp/fonts/README.md)
- [Tips on designing an app UI](doc/ui_guidelines.md)
- [Bootloader, OTA and DFU](bootloader/README.md)
- [External resources](doc/ExternalResources.md)

### Contributing

- [How to contribute](CONTRIBUTING.md)
- [Coding conventions](doc/coding-convention.md)

### Build, flash and debug

- [InfiniTime simulator](https://github.com/InfiniTimeOrg/InfiniSim)
- [Build the project](doc/buildAndProgram.md)
- [Build the project with Docker](doc/buildWithDocker.md)
- [Build the project with VSCode](doc/buildWithVScode.md)
- [Flash the firmware using OpenOCD and STLinkV2](doc/openOCD.md)
- [Flash the firmware using SWD interface](doc/SWD.md)
- [Flash the firmware using JLink](doc/jlink.md)
- [Flash the firmware using GDB](doc/gdb.md)
- [Stub using NRF52-DK](doc/PinetimeStubWithNrf52DK.md)

### API

- [BLE implementation and API](doc/ble.md)

### Architecture and technical topics

- [Memory analysis](doc/MemoryAnalysis.md)

### Project management

- [Maintainer's guide](doc/maintainer-guide.md)
- [Versioning](doc/versioning.md)
- [Project branches](doc/branches.md)
- [Files included in the release notes](doc/filesInReleaseNotes.md)
- [Files needed by the factory](doc/files-needed-by-factory.md)

## Licenses

This project is released under the GNU General Public License version 3 or, at your option, any later version.

It integrates the following projects:

- RTOS: **[FreeRTOS](https://freertos.org)** under the MIT license
- UI: **[LittleVGL/LVGL](https://lvgl.io/)** under the MIT license
- BLE stack: **[NimBLE](https://github.com/apache/mynewt-nimble)** under the Apache 2.0 license
- Font: **[Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)** under the Apache 2.0 license

## Credits

I’m not working alone on this project. First, many people create pull requests for this project. Then, there is the whole #pinetime community: a lot of people all around the world who are hacking, searching, experimenting and programming the Pinetime. We exchange our ideas, experiments and code in the chat rooms and forums.

Here are some people I would like to highlight:

- [Atc1441](https://github.com/atc1441/): He works on an Arduino based firmware for the Pinetime and many other smartwatches based on similar hardware. He was of great help when I was implementing support for the BMA421 motion sensor and I²C driver.
- [Koen](https://github.com/bosmoment): He’s working on a firmware based on RiotOS. He integrated similar libs as me: NimBLE, LittleVGL,… His help was invaluable too!
- [Lup Yuen Lee](https://github.com/lupyuen): He is everywhere: he works on a Rust firmware, builds a MCUBoot based bootloader for the Pinetime, designs a Flutter based companion app for smartphones and writes a lot of articles about the Pinetime!
