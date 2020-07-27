# Using the releases
For each new *stable* version of Pinetime, a [release note](https://github.com/JF002/Pinetime/releases) is created. It contains a description of the main changes in the release and some files you can use to flash the firmware in your Pinetime.

This page describes the files from the release notes and how to use them.

**NOTE :** the files included in different could be different. This page describes the release note of [version 0.7.1](https://github.com/JF002/Pinetime/releases/tag/0.7.1), which is the version that'll probably be pre-programmed at the factory for the next batch of Pinetime devkits.

## Files included in the release note

### Standalone firmware
This firmware is standalone, meaning that it does not need a bootloader to actually run. It is intended to be flash at offset 0, meaning it will erase any bootloader that might be present in memory.

 - **pinetime-app.out** : Output file of GCC containing debug symbols, useful is you want to debug the firmware using GDB.
 - **pinetime-app.hex** : Firmware in Intel HEX file format. Easier to use because it contains the offset in memory where it must be flashed, you don't need to specify it.
 - **pintime-app.bin** : Firmware in binary format. When programming it, you have to specify the offset (0x00) in memory where it must be flashed.
 - **pinetime-app.map** : Map file containing all the symbols, addresses in memory,...
 
**This firmware must be flashed at address 0x00 in the main flash memory**

### Bootloader
The bootloader  is maintained by [lupyuen](https://github.com/lupyuen) and is a binary version of [this release](https://github.com/lupyuen/pinetime-rust-mynewt/releases/tag/v4.1.7).

 - **bootloader.hex** : Firmware in Intel HEX file format.
 
 **This firmware must be flashed at address 0x00 in the main flash memory**


### Graphics firmware 
This firmware is a small utility firmware that writes the boot graphic in the external SPI flash memory. You need it if you want to use the [bootloader](../bootloader/README.md).

 - **pinetime-graphics.out** : Output file of GCC containing debug symbols, useful is you want to debug the firmware using GDB.
 - **pinetime-graphics.hex** : Firmware in Intel HEX file format. Easier to use because it contains the offset in memory where it must be flashed, you don't need to specify it.
 - **pintime-graphics.bin** : Firmware in binary format. When programming it, you have to specify the offset (0x00) in memory where it must be flashed.
 - **pinetime-graphics.map** : Map file containing all the symbols, addresses in memory,...
 
**This firmware must be flashed at address 0x00 in the main flash memory**

### Firmware with bootloader
This firmware is intended to be used with our [MCUBoot-based bootloader](../bootloader/README.md).

 - **pinetime-mcuboot-app-image.hex** : Firmware wrapped into an MCUBoot image. This is **the** file that must be flashed **@ 0x8000** into flash memory. If the [bootloader](../bootloader/README.md) has been successfully programmed, it should run this firmware after the next reset.

The following files are not directly usable by the bootloader:

 - **pinetime-mcuboot-app.bin** : Output file of GCC containing debug symbols, useful is you want to debug the firmware using GDB.
 - **pinetime-mcuboot-app.hex** : Firmware in Intel HEX file format. 
 - **pinetime-mcuboot-app.bin** : Firmware in binary format. 
 - **pinetime-mcuboot-app.map** : Map file containing all the symbols, addresses in memory,...

### OTA (Update the firmware Over-The-Air)
Once the bootloader and application firmware are running, it is possible to update the current firmware or even replace it with another firmware **that uses the same bootloader based on MCUBoot**.

**NOTE :** Use this file **only** if you programmed our [MCUBoot-based bootloader](../bootloader/README.md). This file is not compatible with other bootloaders like the one that is based on the closed source NRF SoftDevice !

 - **pinetime-app-dfu.zip** : This is the file you must provide toNRFConnect to update the firmware over BLE.

