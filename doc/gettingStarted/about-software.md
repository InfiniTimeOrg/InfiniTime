# Firmware, InfiniTime, Bootloader, Recovery firmware, OTA, DFU... What is it?

You may have already encountered these words by reading the announcement, release notes, or [the wiki guide](https://wiki.pine64.org/wiki/Upgrade_PineTime_to_InfiniTime_1.0.0) and you may find them confusing if you're not familiar with the project.

A **firmware** is software running on the embedded hardware of a device.

InfiniTime has three distinct firmwares:

 - **[InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime)** is the operating system.
 - **[The bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader)** is responsible for safely applying firmware updates and runs before booting into InfiniTime.
 - **The recovery firmware** is a special *application firmware* than can be loaded by the bootloader on user request. This firmware can be useful in case of serious issue, when the main application firmware cannot perform an OTA update correctly.

**OTA** (**O**ver **T**he **A**ir) refers to updating of the firmware over BLE (**B**luetooth **L**ow **E**nergy). This is a functionality that allows the user to update the firmware on their device wirelessly.

**DFU** (**D**evice **F**irmware **U**pdate) is the file format and protocol used to send the update of the firmware to the watch over-the-air. InfiniTime implement the (legacy) DFU protocol from Nordic Semiconductor (NRF).

## Bootloader

Most of the time, the bootloader just runs without your intervention (update and load the firmware).

However, you can enable 2 functionalities using the push button:

 - Push the button until the pine cone is drawn in **blue** to force the rollback of the previous version of the firmware, even if you've already validated the updated one
 - Push the button until the pine cone is drawn in **red** to load the recovery firmware. This recovery firmware only provides BLE connectivity and OTA functionality.

More info about the bootloader in [its project page](https://github.com/JF002/pinetime-mcuboot-bootloader/blob/master/README.md).
