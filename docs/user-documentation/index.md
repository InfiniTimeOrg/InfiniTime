# User documentation

This part of the documentation is meant for end users.   
For developer documentation, please refer to [this page](/developer-documentation/index.html)

## Getting started with InfiniTime

### Unboxing your PineTime

The box contains:

- the PineTime,
- a USB-A charger/craddle,
- a quick user guide.

### Boot/Reboot/Switch off InfiniTime

The PineTime has a single button, located on the left hand side.   

- To start/boot your PineTime, simply hold the button for a few seconds until
  the Pine64 logo appears,
  - if nothing happens, you may have to charge the watch beforehand.
- To reboot/restart the watch, press and hold the button for approximately 8
  seconds
  - Release the button at that stage, otherwise you will trigger another action
    (see [Recovery
    firmware](index.html#firmware-bootloader-recovery-firmware) and [Firmware
    validation](index.html#firmware-validation)
    for more details).
- It is **not** possible to switch it off.

### Setting up date and time

By default, InfiniTime starts on the digital watchface. It'll probably display
the epoch time (1 Jan 1970, 00:00).

You can set the time (and date) manually, or have a companion app do it for you.

InfiniTime doesn't handle daylight savings automatically, so make sure to set
the correct time or sync with a companion app.

#### Using companion apps

Date and time are set by the companion app once the PineTime is connected over
BLE.

#### Manually

Starting with InfiniTime 1.7, it is possible to configure the date and
time directly from the watch.

This can be done from the Settings menu, then "Set date" and "Set time"
respectively:

(insert here pics of IT)

#### Using any Chromium-based web-browser

You can use [WebBLE](https://hubmartin.github.io/WebBLEWatch/) from a
Chromium-based browser (Chrome, Chromium, Edge, Chrome Android) to setup the
date and time.

(insert here pics of WebBLE GH)

#### Using NRFConnect

You must enable the **CTS** *GATT* server into NRFConnect so that InfiniTime can
synchronize the time with your smartphone.

Launch NRFConnect, tap the sandwich button on the top left and select *Configure
GATT server*:

Tap *Add service* and select the server configuration *Current Time service*.
Tap OK and connect to your PineTime, it should automcatically sync the time once
the connection is established!

(insert pics from
https://github.com/InfiniTimeOrg/InfiniTime/blob/develop/doc/gettingStarted/ota-gadgetbridge-nrfconnect.md#using-nrfconnect-1)

### Companion apps

PineTime can be used as a standalone watch, displaying date and time (which can
be configured from the watch itself since InfiniTime 1.7.0), as well as
heart-rate, number of steps, used as a torchlight, or just to play the included
games (2048, Pong, and Draw).   

To get more features, Companion apps, which are applications running on a
smartphone or a computer, and are paired to the PineTime, are required.   

There are multiple Companion apps available:

- Smartphones:
  - Android: [GadgetBridge](https://gadgetbridge.org/)
  - SailfishOS: [Amazfish](https://openrepos.net/content/piggz/amazfish/)
  - iOS: [InfiniLink](https://github.com/xan-m/InfiniLink)
  - PinePhone (Linux phone): [Siglo](https://github.com/alexr4535/siglo)
- Linux Computer:
  - [Amazfish](https://openrepos.net/content/piggz/amazfish/) and
    [Siglo](https://github.com/alexr4535/siglo) will also work, but may require
    some manual installation.

### The InfiniTime UI

The UI is composed of 4 different areas:
- the main watchface
- the notification screen (swipe down)
- apps drawer (swipe up)
- quick settings (swipe right)

#### Watchfaces

The default watchface is the "digital one", which displays date and time, as
well as the number of steps, heart-rate, bluetooth icon (when connected),
battery status, and possibly missed notifications.

There are 2 other watchfaces:

- Analog 
- PineTimeStyle

(Add pics from the watchfaces)

#### Notification screen

When swiping down, the last notification is displayed.   
Up to 5 notifications can be displayed: simply swipe up again the display the
next notification.   
To come back to the watchface, press the left button.

It is currently not possible to discard notifications, unless you restart
InfiniTime (long press on the button for ~8 seconds).

(Insert notification pics)

#### Apps drawer

When swiping up, the apps drawer allows launching applications.

There are 2 pages with each 6 applications (2 rows of 3 apps):

Page 1:
- Stopwatch
- Music control
- Navigation (only works with PureMaps/Sailfish OS)
- Steps counter
- Heart-rate
- Countdown 

Page 2:
- Draw
- Pong game
- 2048 game
- Accelerometer
- Metronome
- Alarm

(Add pics of the 2 pages of apps)

**(Note to JF: does it make sense to have a subsection for each app, with
screenshots and more details?)**

#### Quick settings

When swiping right, you get access to 4 icons - from top left to bottom right:

- brightness level: pressing it will cycle between 3 levels (low/medium/high)
- torch: 
  - a tap launches it, 
  - another tap switches it on, 
  - another one switches it off,
  - swiping to the right or left changes the brightness level of the torch
- silent mode:
  - a green bell symbol means silent mode is off (so the watch will vibrate when
    receiving a notification),
  - taping it enables it: the icon becomes grey, and the watch will not vibrate
    when receiving notifications)
- settings: access to InfiniTime settings
  - Display timeout (in seconds)
  - Wake up: how to wake up the watch
    - nothing selected means only the left button wakes up the watch
    - single tap: tap one time on the screen to wake up the watch
    - double tap: tap two times to wake it up
    - raise wrist: screen will wake up when you raise your wrist
- Time format: 12h or 24h
- Watch face: choose between digital, analog and PineTimeStyle
- Steps: define your daily goal
- Set date: allows manually setting date
- Set time: allows manually setting time
- Battery: displays battery level and voltage
- PTS Colors: allows changing colors for the PineTimeStyle watchface
- Firmware: displays information about the InfiniTime version
- About: displays information about InfiniTime, the Bootloader, uptime, etc

## Flash And Upgrade

### Bootloader, Firmware and recovery firmware

You can check the InfiniTime version by first swiping right on the watchface to
open quick settings, tapping the cogwheel to open settings, swipe up until you
find an entry named "About" and tap on it.

(add link to
https://github.com/InfiniTimeOrg/InfiniTime/raw/develop/doc/gettingStarted/version-1.0.jpg)

#### Bootloader

The
[bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader/releases/tag/1.0.0)
is run right before booting into InfiniTime.   
It is easily recognizable with its white pine cone that is progressively drawn
in green. It also displays its own version on the bottom (1.0.0 as of now).

(add link to
https://github.com/InfiniTimeOrg/InfiniTime/blob/develop/doc/gettingStarted/bootloader-1.0.jpg)

#### The firmware

Well, it's InfiniTime :) 

#### Recovery firmware

The *recovery functionality* allows to load a [recovery
firmware](https://github.com/InfiniTimeOrg/InfiniTime/releases/tag/0.14.1) from
the external flash memory to recover the PineTime when the current firmware
cannot boot anymore. 

This recovery firmware is a slightly modified version of InfiniTime that only
provides a basic UI and the OTA functionality. You'll be able to use this
firmware to load a new firmware over-the-air using BLE connectivity.

[This
article](https://www.ncartron.org/pinetimes-infinitime-new-bootloader-and-a-recovery-firmware.html)
describes how to upgrade your PineTime to benefit from this feature.

PineTime units shipped after (confirm date with JF) come with the recovery
firmware already installed, so there's no need to follow this procedure.

### Upgrading your PineTime
#### Over-The-Air (OTA)
#### Using Gadgetbridge
#### Using Amazfish
#### Using NRFConnect
### Using the SWD interface
## Firmware validation
