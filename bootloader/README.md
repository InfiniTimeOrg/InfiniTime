# About this bootloader
This bootloader is mostly developed by [Lup Yuen](https://github.com/lupyuen). It is based on MCUBoot and Mynewt.

The goal of this project is to provide a common bootloader for multiple (all?) Pinetime projects. It allows to upgrade the current bootloader and even replace the current application by another one that supports the same bootloader.

As we wanted this bootloader to be as universal as possible, we decided that it should **not** integrate a BLE stack and provide OTA capabilities. 

Integrating a BLE stack for the OTA functionality would have used to much memory space and/or forced all the firmware developers to use the same BLE stack as the bootloader.

When it is run, this bootloader looks in the SPI flash memory if a new firmware is available. It there is one, it *swaps* the current firmware with the new one (the new one is copied in the main flash memory, and the current one is copied in the SPI flash memory) and run the new one. If the new one fails to run properly, the bootloader is able to revert to the old one and mark the new one as not working.

As this bootloader does not provide any OTA capability, it is not able to actually download a new version of the application. Providing OTA functionality is thus the responsability of the application firmware.

# Using the bootlader

## Bootloader graphic
The bootloader loads a graphic (Pinetime logo) from the SPI Flash memory. If this graphic is not loaded in the memory, the LCD will display garbage (the content of the SPI flash memory).

The SPI Flash memory is not accessible via the SWD debugger. Use the firmware 'pinetime-graphics' to load the graphic into memory. All you have to do is build it and program it at address 0x00 :

 - Build:
```
$ make pinetime-graphics
```

 - Program (using OpenOCD for example) : 
```
program pinetime-graphics.bin 0
```

 - Let it run for ~10s (it does nothing for 5 seconds, then write the logo into the SPI memory, then (slowly) displays it on the LCD).

## Bootloader binary
The binary comes from https://github.com/lupyuen/pinetime-rust-mynewt/releases/tag/v4.1.7

It must be flash at address **0x00** in the internal flash memory.

Using OpenOCD:

`
program mynewt_nosemi.elf_4.1.7.bin 0
`

## Application firmware image
Build the binary compatible with the booloader:

`
make pinetime-mcuboot-app
`

The binary is located in *<build directory>/src/pinetime-mcuboot-app.bin*.  

It must me converted into a MCUBoot image using *imgtool.py* from [MCUBoot](https://github.com/JuulLabs-OSS/mcuboot/tree/master/scripts). Simply checkout the project and run the script <mcuboot root>/scripts/imgtool.py with the following command line:

`
imgtool.py  create --align 4 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header <build directory>/src/pinetime-mcuboot-app.bin image.bin
`

The image must be then flashed at address **0x8000** in the internal flash memory.

Using OpenOCD:

`
program image.bin 0x8000
`

## OTA and DFU
Pack the image into a .zip file for the NRF DFU protocol:

`
adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application image.bin dfu.zip
`

Use NRFConnect or dfu.py (in <project root>/bootloader/ota-dfu-python) to upload the zip file to the device:

`
sudo dfu.py -z /home/jf/nrf52/bootloader/dfu.zip -a <pinetime MAC address> --legacy
`

**Note** : dfu.py is a slightly modified version of [this repo](https://github.com/daniel-thompson/ota-dfu-python).