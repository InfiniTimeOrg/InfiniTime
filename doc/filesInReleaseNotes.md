# Using the releases

For each new *stable* version of IniniTime, a [release note](https://github.com/InfiniTimeOrg/InfiniTime/releases) is created. It contains a description of the main changes in the release and some files you can use to flash the firmware to your Pinetime.

This page describes the files from the release notes and how to use them.

**NOTE :** the files included in different Releases could be different. This page describes the release notes of [version 0.7.1](https://github.com/InfiniTimeOrg/InfiniTime/releases/tag/0.7.1), which is the version that is pre-programmed for the first batches of pinetimes but has been replaced with [1.0.0](https://github.com/InfiniTimeOrg/pinetime-mcuboot-bootloader/releases/tag/1.0.0) since around june 2021.

## Files included in the release notes

### Standalone firmware

This firmware is standalone, meaning that it does not need a bootloader to actually run. It is intended to be flashed at offset 0, meaning it will erase any bootloader that might be present in memory.

- **pinetime-app.out** : Output file of GCC containing debug symbols, useful if you want to debug the firmware using GDB.
- **pinetime-app.hex** : Firmware in Intel HEX file format. Easier to use because it contains the offset in memory where it must be flashed, you don't need to specify it.
- **pintime-app.bin** : Firmware in binary format. When programming it, you have to specify the offset (0x00) in memory where it must be flashed.
- **pinetime-app.map** : Map file containing all the symbols, addresses in memory,...

**This firmware must be flashed at address 0x00 in the main flash memory**

### Bootloader

The bootloader is maintained by [lupyuen](https://github.com/lupyuen) and is a binary version of [this release](https://github.com/lupyuen/pinetime-rust-mynewt/releases/tag/v5.0.4).

- **bootloader.hex** : Firmware in Intel HEX file format.

**This firmware must be flashed at address 0x00 in the main flash memory**

### Firmware with bootloader

This firmware is intended to be used with our [MCUBoot-based bootloader](https://github.com/InfiniTimeOrg/pinetime-mcuboot-bootloader/tree/develop).

- **pinetime-mcuboot-app-image.hex**: Firmware wrapped into an MCUBoot image. This is **the** file that must be flashed at **0x8000** into the flash memory. If the [bootloader](https://github.com/InfiniTimeOrg/pinetime-mcuboot-bootloader/tree/develop) has been successfully programmed, it should run this firmware after the next reset.

The following files are not directly usable by the bootloader:

- **pinetime-mcuboot-app.out** : Output file of GCC containing debug symbols, useful is you want to debug the firmware using GDB.
- **pinetime-mcuboot-app.hex** : Firmware in Intel HEX file format.
- **pinetime-mcuboot-app.bin** : Firmware in binary format.
- **pinetime-mcuboot-app.map** : Map file containing all the symbols, addresses in memory,...

### OTA (Update the firmware Over-The-Air)

Once the bootloader and application firmware are running, it is possible to update the current firmware or even replace it with another firmware **that uses the same bootloader based on MCUBoot**.

**NOTE :** Use this file **only** if you programmed our [MCUBoot-based bootloader](https://github.com/InfiniTimeOrg/pinetime-mcuboot-bootloader/tree/develop). This file is not compatible with other bootloaders like the one that is based on the closed source NRF SoftDevice !

- **pinetime-app-dfu.zip** : This is the file you must provide to the [companion app](https://github.com/InfiniTimeOrg/InfiniTime#companion-apps) to update the firmware over BLE.
