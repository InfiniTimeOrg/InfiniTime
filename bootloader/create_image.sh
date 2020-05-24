#!/bin/bash

/home/jf/nrf52/mcuboot/scripts/imgtool.py  create --align 4 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header /home/jf/nrf52/Pinetime/cmake-build-release/src/pinetime-mcuboot-app.bin image.bin