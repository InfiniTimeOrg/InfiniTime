#!/bin/bash
(return 0 2>/dev/null) && SOURCED="true" || SOURCED="false"
export LC_ALL=C.UTF-8
export LANG=C.UTF-8
set -x
set -e

# Default locations if the var isn't already set
export TOOLS_DIR="${TOOLS_DIR:=/opt}"
export SOURCES_DIR="${SOURCES_DIR:=/sources}"
export BUILD_DIR="${BUILD_DIR:=$SOURCES_DIR/build}"
export OUTPUT_DIR="${OUTPUT_DIR:=$SOURCES_DIR/build/output}"

export BUILD_TYPE=${BUILD_TYPE:=Release}
export GCC_ARM_VER=${GCC_ARM_VER:="11.2-2022.02"}
export NRF_SDK_VER=${NRF_SDK_VER:="nRF5_SDK_15.3.0_59ac345"}

MACHINE="$(uname -m)"
[[ "$MACHINE" == "arm64" ]] && MACHINE="aarch64"

export GCC_ARM_PATH="gcc-arm-$GCC_ARM_VER-$MACHINE-arm-none-eabi"

main() {
  local target="$1"

  mkdir -p "$TOOLS_DIR"

  [[ ! -d "$TOOLS_DIR/$GCC_ARM_PATH" ]] && GetGcc
  [[ ! -d "$TOOLS_DIR/$NRF_SDK_VER" ]] && GetNrfSdk
  [[ ! -d "$TOOLS_DIR/mcuboot" ]] && GetMcuBoot

  mkdir -p "$BUILD_DIR"

  CmakeGenerate
  CmakeBuild $target
  BUILD_RESULT=$?
  if [ "$DISABLE_POSTBUILD" != "true" -a "$BUILD_RESULT" == 0 ]; then
    source "$BUILD_DIR/post_build.sh"
  fi
}

GetGcc() {
  wget -q https://developer.arm.com/-/media/Files/downloads/gnu/$GCC_ARM_VER/binrel/$GCC_ARM_PATH.tar.xz -O - | tar -xJ -C $TOOLS_DIR/
}

GetMcuBoot() {
  git clone https://github.com/mcu-tools/mcuboot.git "$TOOLS_DIR/mcuboot"
  pip3 install -r "$TOOLS_DIR/mcuboot/scripts/requirements.txt"
}

GetNrfSdk() {
  wget -q "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/$NRF_SDK_VER.zip" -O /tmp/$NRF_SDK_VER
  unzip -q /tmp/$NRF_SDK_VER -d "$TOOLS_DIR/"
  rm /tmp/$NRF_SDK_VER
}

CmakeGenerate() {
  cmake -G "Unix Makefiles" \
    -S "$SOURCES_DIR" \
    -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DUSE_OPENOCD=1 \
    -DARM_NONE_EABI_TOOLCHAIN_PATH="$TOOLS_DIR/$GCC_ARM_PATH" \
    -DNRF5_SDK_PATH="$TOOLS_DIR/$NRF_SDK_VER" \
    -DBUILD_DFU=1
}

CmakeBuild() {
  local target="$1"
  [[ -n "$target" ]] && target="--target $target"
  if cmake --build "$BUILD_DIR" --config $BUILD_TYPE $target -- -j$(nproc)
    then return 0; else return 1; 
  fi
}

[[ $SOURCED == "false" ]] && main "$@" || echo "Sourced!"
