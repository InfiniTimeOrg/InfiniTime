# Bootloader

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