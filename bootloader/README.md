# Bootloader

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

It must me converted into a MCUBoot image using *imgtool.py* from [MCUBoot](https://github.com/JuulLabs-OSS/mcuboot/tree/master/scripts).

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