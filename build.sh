#!/bin/bash

mkdir -p build
cd build

echo "Configuring build..."
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH="/workspaces/infinitime/gcc-arm" -DNRF5_SDK_PATH="/workspaces/infinitime/nRF5_SDK" -DCMAKE_BUILD_TYPE="Release" -DBUILD_DFU=1 -DBUILD_RESOURCES=1 -DTARGET_DEVICE="PINETIME" ..
echo "Finished configuring build"

echo "Building..."
make -j4 pinetime-mcuboot-app
echo "Finished building"
