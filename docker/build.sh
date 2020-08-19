#!/bin/sh
export LC_ALL=C.UTF-8
export LANG=C.UTF-8

mkdir /sources/build
cd /sources/build

cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-9-2020-q2-update -DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345 -DUSE_OPENOCD=1 ../
make -j

/opt/mcuboot/scripts/imgtool.py create --align 4 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header /sources/build/src/pinetime-mcuboot-app.bin /sources/build/image.bin
adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application /sources/build/image.bin /sources/build/dfu.zip

mkdir -p /sources/build/output
mv /sources/build/image.bin /sources/build/output/pinetime-mcuboot-app.img
mv /sources/build/dfu.zip /sources/build/output/pinetime-app-dfu.zip

cp /sources/build/src/*.bin /sources/build/output/
cp /sources/build/src/*.hex /sources/build/output/
cp /sources/build/src/*.out /sources/build/output/
cp /sources/build/src/*.map /sources/build/output/
cp /sources/bootloader/mynewt_nosemi_4.1.7.elf.bin /sources/build/output/bootloader.bin



