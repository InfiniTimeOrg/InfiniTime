#!/bin/sh
export LC_ALL=C.UTF-8
export LANG=C.UTF-8
set -x

mkdir /sources/build
cd /sources/build

cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-9-2020-q2-update -DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345 -DUSE_OPENOCD=1 ../
make -j$(nproc)

sh /sources/docker/post_build.sh
