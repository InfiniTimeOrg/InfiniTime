## Firmware, InfiniTime, Bootloader, Recovery firmware, OTA, DFU... What is it?

You may have already encountered these words by reading the announcement, release notes, or [the wiki guide](https://wiki.pine64.org/wiki/Upgrade_PineTime_to_InfiniTime_1.0.0) and you may find them confusing if you're not familiar with the project.

A **firmware** is software running on the embedded hardware of a device.

InfiniTime has three distinct firmwares:

 - **[InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime)** is the operating system.
 - **[The bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader)** is responsible for safely applying firmware updates and runs before booting into InfiniTime.
 - **The recovery firmware** is a special *application firmware* than can be loaded by the bootloader on user request. This firmware can be useful in case of serious issue, when the main application firmware cannot perform an OTA update correctly.

**OTA** (**O**ver **T**he **A**ir) refers to updating of the firmware over BLE (**B**luetooth **L**ow **E**nergy). This is a functionality that allows the user to update the firmware on their device wirelessly.

**DFU** (**D**evice **F**irmware **U**pdate) is the file format and protocol used to send the update of the firmware to the watch over-the-air. InfiniTime implement the (legacy) DFU protocol from Nordic Semiconductor (NRF).

## How to check the version of InfiniTime and the bootloader?

You can check the InfiniTime version by first swiping right on the watchface to open quick settings, tapping the cogwheel to open settings, swipe up until you find an entry named "About" and tap on it.

![InfiniTime 1.0 version](version-1.0.jpg)

PineTimes shipped after June 2021 will be flashed with the [new version of the bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader/releases/tag/1.0.0), the [recovery firmware](https://github.com/InfiniTimeOrg/InfiniTime/releases/tag/0.14.1) and [InfiniTime 1.0](https://github.com/InfiniTimeOrg/InfiniTime/releases/tag/1.0.0).

The bootloader is run right before booting to InfiniTime.

The bootloader is easily recognizable with its white pine cone that is progressively drawn in green. It also displays its own version on the bottom (1.0.0 as of now).

![Bootloader 1.0](bootloader-1.0.jpg)

## How to update your PineTime?

To update your PineTime, you can use one of the compatible companion applications.

The updating process differs slightly on every companion app, so you'll need to familiarize yourself with the companion app of your choice.

All releases of InfiniTime are available on the [release page of the GitHub repo](https://github.com/InfiniTimeOrg/InfiniTime/releases) under assets.

To update the firmware, you need to download the DFU of the firmware version that you'd like to install, for example `pinetime-mcuboot-app-dfu-1.6.0.zip`, and flash it with your companion app.

We have prepared instructions for flashing InfiniTime with Gadgetbridge and NRFConnect.

 - [Updating with Gadgetbridge](/doc/gettingStarted/ota-gadgetbridge.md)
 - [Updating with NRFConnect](/doc/gettingStarted/ota-nrfconnect.md)

### Firmware validation

Firmware updates must be manually validated. If the firmware isn't validated and the watch resets, the watch will revert to the previous firmware. This is a safety feature to prevent bricking your device with faulty firmware.

You can validate your updated firmware on InfiniTime >= 1.0 by following this simple procedure:

 - From the watchface, swipe **right** to display the *quick settings menu*
 - Open settings by tapping the cogwheel on the bottom right
 - Swipe up until you find an entry named **Firmware** and tap on it
 - If the firmware is not validated yet, you can either validate the running firmware, or reset and revert to the previous firmware version

## Bootloader

Most of the time, the bootloader just runs without your intervention (update and load the firmware).

However, you can enable 2 functionalities using the push button:

 - Push the button until the pine cone is drawn in **blue** to force the rollback of the previous version of the firmware, even if you've already validated the updated one
 - Push the button until the pine cone is drawn in **red** to load the recovery firmware. This recovery firmware only provides BLE connectivity and OTA functionality.

More info about the bootloader in [its project page](https://github.com/JF002/pinetime-mcuboot-bootloader/blob/master/README.md).
