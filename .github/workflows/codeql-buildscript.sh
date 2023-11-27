#!/usr/bin/env bash

set -e

sudo apt-get install -y python3-venv python3-pip
npm install lv_font_conv

wget --no-verbose 'https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip'
unzip -q nRF5_SDK_15.3.0_59ac345

python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install wheel
python3 -m pip install -r tools/mcuboot/requirements.txt

cmake -Bbuild -DNRF5_SDK_PATH=$(realpath nRF5_SDK_15.3.0_59ac345) \
  -DARM_NONE_EABI_TOOLCHAIN_PATH=$(dirname $(dirname $(realpath $(which arm-none-eabi-gcc))))
cmake --build build -t pinetime-app
