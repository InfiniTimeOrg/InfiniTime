# User documentation

This part of the documentation is meant for end users.   
For developer documentation, please refer to [this page](developer-documentation/index.html)

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
### The InfiniTime UI
## Flash And Upgrade
### Firmware, bootloader, recovery firmware
### Upgrading your PineTime
#### Over-The-Air (OTA)
#### Using Gadgetbridge
#### Using Amazfish
#### Using NRFConnect
### Using the SWD interface
## Firmware validation
