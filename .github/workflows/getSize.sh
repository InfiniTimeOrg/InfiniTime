#!/bin/sh

# Requires environment variables from docker/build.sh

set -e

SIZE_BIN="$TOOLS_DIR/$GCC_ARM_PATH/bin/arm-none-eabi-size"
[ ! -x "$SIZE_BIN" ] && exit 1

[ -z "$1" ] && exit 1
SIZE_OUTPUT=$($SIZE_BIN "$1" | tail -n1)

TEXT_SIZE=$(echo "$SIZE_OUTPUT" | cut -f 1 |tr -d '[:blank:]')
DATA_SIZE=$(echo "$SIZE_OUTPUT" | cut -f 2 |tr -d '[:blank:]')
BSS_SIZE=$(echo "$SIZE_OUTPUT" | cut -f 3 |tr -d '[:blank:]')

echo "text_size=$TEXT_SIZE"
echo "data_size=$DATA_SIZE"
echo "bss_size=$BSS_SIZE"
