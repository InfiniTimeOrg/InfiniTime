# PineTime
> The PineTime is a free and open source smartwatch capable of running custom-built open operating systems. Some of the notable features include a heart rate monitor, a week-long battery as well as a capacitive touch IPS display that is legible in direct sunlight. It is a fully community driven side-project, which means that it will ultimately be up to the developers and end-users to determine when they deem the PineTime ready to ship.

> We envision the PineTime as a companion for not only your PinePhone but also for your favorite devices — any phone, tablet, or even PC.

*https://www.pine64.org/pinetime/*

The **Pinetime** smartwatch is based built around the NRF52832 MCU (512KB Flash, 64KB Ram), a 240*240 LCD display driven by the ST7789 controller, an accelerometer, a heartrate sensor and a vibrator.

The goal of this project is to design an open-source firmware for the Pinetime smartwatch : 

 - Code written in **modern C++**;
 - Build system based on **CMake**;
 - Based on **[FreeRTOS](https://freertos.org)** real-time OS.
 - Using **[LittleVGL/LVGL](https://lvgl.io/)** as UI library...
 - ... and **[NimBLE](https://github.com/apache/mynewt-nimble)** as BLE stack.

##Overview

![Pinetime screens](images/0.7.0/montage.jpg "PinetimeScreens")

As of now, here is the list of achievements of this project:

 - Fast and optimized LCD driver
 - BLE communication
 - Rich user interface via display, touchpanel and push button
 - Time synchronisation via BLE
 - Notification via BLE
 - Multiples 'apps' : 
    * Clock (displays the date, time, battery level, ble connection status, heart rate and step count)
    * Heart rate
    * Motion
    * System info (displays various info : BLE MAC, build date/time, uptime, version,...)
    * Brightess (allows the user to configure the brightness of the display)
 - Supported by 2 companion apps (developpments ongoing):
    * [Gadgetbridge](https://codeberg.org/Freeyourgadget/Gadgetbridge/src/branch/pinetime-jf) (on Android)
    * [Amazfish](https://openrepos.net/content/piggz/amazfish) (on SailfishOS)
 - **[Experimental]** OTA (Over-the-air) update via BLE
 - **[Experimental]** Bootloader based on [MCUBoot](https://juullabs-oss.github.io/mcuboot/)
 
## Documentation

 - Build & program
 - Bootloader, OTA and DFU
 - Stub using NRF52-DK
 - BLE implementation and API
 - Memory analysis
    
## TODO - contribute

This project is far from beeing finished, and there are still a lot of things to do for this project to become a firmware usable by the general public.

Here a quick list out of my head of things to do for this project:

 - Improve BLE communication stability and reliability
 - Improve OTA and MCUBoot bootloader
 - Add more functionalities : Alarm, chrono, configuration, activities, heart rate logging, games,...
 - Add more BLE functionalities : call notifications, agenda, configuration, data logging,...
 - Measure power consumption and improve battery life
 - Improve documentation, take better pictures and video than mine
 - Improve the UI
 - Create companion app for multiple OSes (Linux, Android, IoS) and platforms (desktop, ARM, mobile). Do not forget the other devices from Pine64 like [the Pinephone](https://www.pine64.org/pinephone/) and the [Pinebook Pro](https://www.pine64.org/pinebook-pro/). 
 - Design a simple CI (preferably selfhosted and easy to reproduce).
 
Do not hesitate to clone/fork the code, hack it and create pull-requests. I'll do my best to review and merge them :)

## Credits 
I’m not working alone on this project. First, many people create PR for this projects. Then, there is the whole #pinetime community : a lot of people all around the world who are hacking, searching, experimenting and programming the Pinetime. We exchange our ideas, experiments and code in the chat rooms and forums.

Here are some people I would like to highlight:

 - Atc1441 : He works on an Arduino based firmware for the Pinetime and many other smartwatches based on similar hardware. He was of great help when I was implementing support for the BMA421 motion sensor and I²C driver.
 - Koen : He’s working on a firmware based on RiotOS. He integrated similar libs than me : NimBLE, LittleVGL,… His help was invaluable too!
 - Lup Yuen Lee : He is everywhere : he works on a Rust firmware, buils a MCUBoot based bootloader for the Pinetime, design a Flutter based companion app for smartphones and write a lot of articles about the Pinetime!

*If you feel like you should appear on this list, just get in touch with me or submit a PR :)** 
