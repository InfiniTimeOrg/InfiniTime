#!/bin/bash

rm -r build

cmake  -DARM_NONE_EABI_TOOLCHAIN_PATH=/home/eve/Work/gcc-arm-none-eabi-10.3-2021.10 -DNRF5_SDK_PATH=/home/eve/Work/nRF5_SDK_17.1.0_ddde560 -DTARGET_DEVICE=PINETIME -DBUILD_DFU=1 -DBUILD_RESOURCES=1 -B build -DCMAKE_BUILD_TYPE=Release

cp make_pine_mcu.sh build/

