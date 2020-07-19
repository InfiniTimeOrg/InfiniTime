#! /bin/bash

cd /pinetime
mkdir build
cd build

if [ $USE_OPENOCD = 1 ]
then
	cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-8-2019-q3-update \
			-DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345\
			-DUSE_OPENOCD=1 \
			-DGDB_CLIENT_TARGET_REMOTE=$GDB_CLIENT \
			../
else
	cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-8-2019-q3-update \
			-DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345\
			-DUSE_JLINK=1 \
			-DGDB_CLIENT_TARGET_REMOTE=$GDB_CLIENT \
			../
fi

make -j pinetime-app
