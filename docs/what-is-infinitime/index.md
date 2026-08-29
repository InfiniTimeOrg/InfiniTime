# What is InfiniTime
## Generalities about PineTime

PineTime is an open source smartwatch produced by
[Pine64](https://www.pine64.org).

From Pine64's website:

"The PineTime is a free and open source smartwatch capable of running
custom-built open operating systems. Some of the notable features include a
heart rate monitor, a week-long battery, and a capacitive touch IPS display that
is legible in direct sunlight. It is a fully community driven side-project which
anyone can contribute to, allowing you to keep control of your device."

More details can be found on the [PineTime page of the Pine64
website](https://www.pine64.org/pinetime/).

## InfiniTime goals

The goal of this project is to design an open-source firmware for the Pinetime
smartwatch :

- Code written in **modern C++**
- Build system based on **CMake**
- Based on [FreeRTOS 10.0.0](https://freertos.org/) real-time OS
- Using [LittleVGL/LVGL 7](https://lvgl.io/) as UI library
- and [NimBLE 1.3.0](https://github.com/apache/mynewt-nimble) as BLE stack

## InfiniTime features - high level

InfiniTime implements the following features:

- Rich user interface via display, touchscreen and pushbutton
- Time synchronization via Bluetooth Low Energy (BLE)
- Time setup via the watch itself
- Notifications via BLE
- Heart rate measurements
- Steps counting
- Wake-up on wrist rotation
- Quick actions
  - Disable vibration on notification
  - Brightness settings
  - Flashlight
  - Settings
- 3 watch faces:
  - Digital
  - Analog
  - [PineTimeStyle](https://wiki.pine64.org/wiki/PineTimeStyle)
- Multiple 'apps' :
  - Music (control the playback of music on your phone)
  - Heart rate (measure your heart rate)
  - Navigation (displays navigation instructions coming from the companion app)
  - Notification (displays the last notification received)
  - Paddle (single player pong-like game)
  - Twos (2048 clone game)
  - Stopwatch
  - Alarm App
  - Steps (displays the number of steps taken)
  - Timer (set a countdown timer that will notify you when it expires)
  - Metronome (vibrates to a given bpm with a customizable beats per bar)
- User settings:
  - Display timeout
  - Wake-up condition
  - Time format (12/24h)
  - Default watch face
  - Daily steps goal
  - Battery status
  - Firmware validation
  - System information
- Supported by multiple companion apps (development is in progress):
  - [Gadgetbridge](https://codeberg.org/Freeyourgadget/Gadgetbridge/) (on Android via F-Droid)
  - [Amazfish](https://openrepos.net/content/piggz/amazfish) (on SailfishOS and Linux)
  - [Siglo](https://github.com/alexr4535/siglo) (on Linux)
  - [ITD](https://gitea.arsenm.dev/Arsen6331/itd/) (on Linux)
  - [Experimental] [WebBLEWatch](https://hubmartin.github.io/WebBLEWatch/) Synchronize time directly from your web browser. [video](https://youtu.be/IakiuhVDdrY)
  - [Experimental] [InfiniLink](https://github.com/xan-m/InfiniLink) (on iOS)
  - OTA (Over-the-air) update via BLE
  - [Bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader) based on
    [MCUBoot](https://www.mcuboot.com/)

## InfiniTime history

### Project history

The project was started by [JF](https://github.com/JF002/) in October 2019.
JF already knew Pine64 as a company that builds ARM-based SBCs, and had
followed the Pinebook Pro and PinePhone.
In September 2019, JF saw an article about the PineTime, an open-source
smartwatch, stating that Pine64 was looking for developers to build a software
running on it.   
After he contacted Pine64 to explain his motivation, he got sent a PineTime dev
kit and starting tinkering with it.

The first version of the bootloader, based on MCUBoot, was written by [Lup
Yuen](https://lupyuen.github.io/).

The first versions of InfiniTime were mainly a one-man show, but quickly a lot
of developers joined the project, and the speed of development and therefore new
features increased a lot:

- The first release, version 0.1.0 already provided basic functionalities :
  display the time, BLE connection and time synchronization. There was no touch
  interface, the UI was slow and basic.
- Version 0.2.2 was the first release that received contributions from other
  developpers :)
  project!
- Version 0.3 switched from a custom graphics library to LittleVGL, a light and
  versatile graphics library for embedded systems. 
- Version 0.5 introduced a change of the BLE stack: instead of using the NRF
  SoftDevice (the BLE stack from Nordic Semiconductor), the decision was taken
  to use [NimBLE](https://github.com/apache/mynewt-nimble), an open source BLE
  stack.   
  Amazfish, a companion app for Sailfish OS and Linux, added support for
  InfiniTime, becoming the first companion app for the smartwatch. 
- Version 0.6 brought the OTA functionality, allowing users to upgrade the
  firmare over-the-air, using their smartphone and BLE connectivity. 
- Version 0.7.1, which brought improvements, optimisations, became the official
  firmware sent with new PineTime (previous firmware was a "work in progress"
  one, and closed-source).

### Firmwares history and changelog

The releases history can be found [here on
GitHub](https://github.com/InfiniTimeOrg/InfiniTime/releases).   

For a list with a summary of new features for each release, please [see this
article](https://www.ncartron.org/pinetimes-infinitime-firmwares-history60.html).
