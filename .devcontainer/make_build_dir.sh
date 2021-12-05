#!/bin/bash
cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release -DUSE_OPENOCD=1 -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-10.3-2021.10 -DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345 ${SOURCES_DIR}
