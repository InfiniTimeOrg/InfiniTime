# The SPI LCD driver

## Introduction

The LCD controller that drives the display of the Pinetime is the [Sitronix ST7789V](https://wiki.pine64.org/images/5/54/ST7789V_v1.6.pdf). This controller is easy to integrate with an MCU thanks to its SPI interface, and has some interesting features like:

- an on-chip display data RAM that can store the whole framebuffer
- partial screen update
- hardware assisted vertical scrolling
- interrupt pin, allowing to drive the display with DMA and IRQ
- ...

When you want to write a device driver for a specific component, its datasheet is your holy bible. This document contains a lot of information about the chip, its specification, characteristics, features and functionalities.
Luckily for us, the datasheet of the ST7789 is great! It contains everything we need to write a nice driver for our beloved Pinetime.

In this document, I'll try to explain the process I've followed to write a device driver for the LCD. There were multiple iterations:

- First, I tried to find the correct initialization sequence so that the controller is configured correctly according to the hardware configuration;
- Then, I tried to display some pixels on the screen;
- Next, I wanted to display squares, colors and text;
- Following, there was a need to make that faster and faster again;
- And finally, I wanted to draw beautiful and useful UIs

I'll describe all these steps in the following chapters.

## The datasheet

As I said in the introduction, the datasheet will be your bedside book during your journey as a device driver designer. You'll read it from the beginning to the end once, twice, maybe ten times. Then, each time you'll want to do something new, you'll reopen the file and search for that specific paragraph or diagram than explains how the controller works so that you can figure out how to use it.

The schematic of your board (the Pinetime schematics in this case) will also be very important, as you'll need to know how the LCD controller is physically connected to the MCU.

How to read the datasheet? I recommend to read it from the beginning to the end (no joke) at least once. You certainly do not need to read everything in details, but it's good to know what information is available and where in the document. It'll be very useful during the development phase.
You'll want to read some part with more attention :

- Data color coding in 4-Line Serial Interface : how to send the pixel to be display to the controller
- Display Data Ram : how is the memory organized
- Power On/Off sequence
- System function commands : all the commands you can send to the controller.

## One Pixel at a time

## Bulk transfers

## DMA

## IRQ

## Bare metal integration

Integration customisée dans la lib GFX que j'ai écrite

## Integration with LittleVGL
