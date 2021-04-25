---
layout: page
title: Build a stub for PineTime using NRF52-DK
---

# Build a stub for PineTime using NRF52-DK
[NRF52-DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52-DK) is the official developpment kit for NRF52832 SoC from Nordic Semiconductor.

It can be very useful for PineTime developpment:
 * You can use it embedded JLink SWD programmer/debugger to program and debug you code on the PineTime
 * As it's based on the same SoC than the PineTime, you can program it to actually run the same code than the PineTime.

This page is about the 2nd point : we will build a stub that will allow us to run the same code than the one you could run on the PineTime. This will allow you to work more easily if you don't have a PineTime dev kit around, if you don't want to modify your dev kit for SWD programming, or if you want to use some feature from the DK (like power measurement).

This stub only implements the display, the button and the BLE radio. The other features from the pintime are missing:
 * heart rate sensor
 * SPI flash
 * touchpad
 * accelerometer

These devices could be added on this stub, but I do not have the parts to try them out for now.

![Pinetime stub](images/pinetimestub1.jpg "PinetimeStub")


Here are the parts you need to build this simulator:
 * [NRF52-DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52-DK)
 * An ST7889 display (I bought [this one](https://www.aliexpress.com/item/32859772356.html?spm=a2g0s.9042311.0.0.1b774c4dSoc4Xz))
 * A push-button (the one I use comes from a previous project build around ESP8266 board Wemos D1 Mini).
 * Dupont wires

You just need to make the following connections:

| NRF52-DK | ST7889 display |
| ---------|--------------- |
| VDD      | VCC            |
| GND      | GND            |
| P0.03    | SDA            |
| P0.26    | RES            |
| P0.02    | SCL            |
| P0.18    | DC             |


| NRF52-DK | Push Button            |
| ---------|----------------------- |
| P0.13 | Button IN (D3 in my case) |
| GND | GND                         |

You also need to enable the I/O expander to disconnect pins from buttons and led on the NRF52-DK and leave them available on the pin headers:

| NRF52 -DK | NRF52- DK |
| --------- | --------- |
| DETECT    | GND       |

Now, you should be able to program the SoC on the NRF52-DK board, and use it as if it was running on the pintime.
