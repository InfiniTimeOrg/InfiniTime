#!/bin/bash

cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OPENOCD=1 -DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345 -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-8-2019-q3-update /source
make
