
# PineTime

[![Build PineTime Firmware](https://github.com/JF002/InfiniTime/workflows/Build%20PineTime%20Firmware/badge.svg?branch=master)](https://github.com/JF002/InfiniTime/actions)

> The PineTime is a free and open source smartwatch capable of running custom-built open operating systems. Some of the notable features include a heart rate monitor, a week-long battery as well as a capacitive touch IPS display that is legible in direct sunlight. It is a fully community driven side-project, which means that it will ultimately be up to the developers and end-users to determine when they deem the PineTime ready to ship.

> We envision the PineTime as a companion for not only your PinePhone but also for your favorite devices — any phone, tablet, or even PC.

*https://www.pine64.org/pinetime/*

The **Pinetime** smartwatch is built around the NRF52832 MCU (512KB Flash, 64KB RAM), a 240*240 LCD display driven by the ST7789 controller, an accelerometer, a heart rate sensor, and a vibration motor.

# InfiniTime
![InfiniTime logo](images/infinitime-logo.jpg "InfiniTime Logo")

The goal of this project is to design an open-source firmware for the Pinetime smartwatch : 

 - Code written in **modern C++**;
 - Build system based on **CMake**;
 - Based on **[FreeRTOS 10.0.0](https://freertos.org)** real-time OS.
 - Using **[LittleVGL/LVGL 7](https://lvgl.io/)** as UI library...
 - ... and **[NimBLE 1.3.0](https://github.com/apache/mynewt-nimble)** as BLE stack.

## Overview

![Pinetime screens](images/1.0.0/collage.png "PinetimeScreens")

As of now, here is the list of achievements of this project:

 - Fast and optimized LCD driver
 - BLE communication
 - Rich user interface via display, touchscreen and pushbutton
 - Time synchronization via BLE
 - Notification via BLE
 - Heart rate measurements
 - Step counting
 - Wake-up on wrist rotation
 - Quick actions 
    * Disable vibration on notification
    * Brightness settings
    * Flashlight
    * Settings
 - 2 watch faces:
    * Digital
    * Analog
 - Multiple 'apps' :
    * Music (control the playback of the music on your phone)
    * Heart rate (controls the heart rate sensor and display current heartbeat)
    * Navigation (displays navigation instructions coming from the companion app)
    * Notification (displays the last notification received)
    * Paddle (single player pong-like game)
    * Two (2048 clone game)
    * Stopwatch (with all the necessary functions such as play, pause, lap, stop)
    * Motion sensor and step counter (displays the number of steps and the state of the motion sensor in real-time)
    * Metronome (vibrates to a given bpm with a customizable beats per bar)
 - User settings:
    * Display timeout
    * Wake-up condition
    * Time format (12/24h)
    * Default watch face
    * Battery status
    * Firmware validation
    * System information
 - Supported by 3 companion apps (development is in progress):
    * [Gadgetbridge](https://codeberg.org/Freeyourgadget/Gadgetbridge/) (on Android via F-Droid)
    * [Amazfish](https://openrepos.net/content/piggz/amazfish) (on SailfishOS and Linux)
    * [Siglo](https://github.com/alexr4535/siglo) (on Linux)
    * **[Experimental]** [WebBLEWatch](https://hubmartin.github.io/WebBLEWatch/) Synchronize time directly from your web browser. [video](https://youtu.be/IakiuhVDdrY)
 - OTA (Over-the-air) update via BLE
 - [Bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader) based on [MCUBoot](https://juullabs-oss.github.io/mcuboot/)
 
## Documentation

### Getting started
 - [Getting started with InfiniTime 1.0 (quick user guide, update bootloader and InfiniTime,...)](doc/gettingStarted/gettingStarted-1.0.md)
 - [Flash, upgrade (OTA), time synchronization,...](doc/gettingStarted/ota-gadgetbridge-nrfconnect.md)

### Develop
 - [Generate the fonts and symbols](src/displayapp/fonts/README.md)
 - [Creating a stopwatch in Pinetime(article)](https://pankajraghav.com/2021/04/03/PINETIME-STOPCLOCK.html)

### Build, flash and debug
 - [Project branches](doc/branches.md)
 - [Versioning](doc/versioning.md)
 - [Files included in the release notes](doc/filesInReleaseNotes.md)
 - [Build the project](doc/buildAndProgram.md)
 - [Flash the firmware using OpenOCD and STLinkV2](doc/openOCD.md)
 - [Build the project with Docker](doc/buildWithDocker.md)
 - [Bootloader, OTA and DFU](./bootloader/README.md)
 - [Stub using NRF52-DK](./doc/PinetimeStubWithNrf52DK.md)
 - Logging with JLink RTT.
 - Using files from the releases

### Contribute
 - [How to contribute ?](doc/contribute.md)

### API
 - [BLE implementation and API](./doc/ble.md)
 
### Architecture and technical topics
 - [Memory analysis](./doc/MemoryAnalysis.md)
 
### Using the firmware
 - [Integration with Gadgetbridge](doc/companionapps/Gadgetbridge.md)
 - [Integration with AmazFish](doc/companionapps/Amazfish.md)
 - [Firmware update, OTA](doc/companionapps/NrfconnectOTA.md)
 
    
## TODO - contribute

This project is far from being finished, and there are still a lot of things to do for this project to become a firmware usable by the general public.

Here a quick list out of my head of things to do for this project:

 - Improve BLE communication stability and reliability
 - Improve OTA and MCUBoot bootloader
 - Add more functionalities : Alarm, chronometer, configuration, activities, heart rate logging, games,...
 - Add more BLE functionalities : call notifications, agenda, configuration, data logging,...
 - Measure power consumption and improve battery life
 - Improve documentation, take better pictures and video than mine
 - Improve the UI
 - Create companion app for multiple OSes (Linux, Android, iOS) and platforms (desktop, ARM, mobile). Do not forget the other devices from Pine64 like [the Pinephone](https://www.pine64.org/pinephone/) and the [Pinebook Pro](https://www.pine64.org/pinebook-pro/). 
 - Design a simple CI (preferably self-hosted and easy to reproduce).
 
Do not hesitate to clone/fork the code, hack it and create pull-requests. I'll do my best to review and merge them :)

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
