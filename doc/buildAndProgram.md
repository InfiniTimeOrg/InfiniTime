# Build

## Dependencies

To build this project, you'll need:

- A cross-compiler : [ARM-GCC (10.3-2021.10)](https://developer.arm.com/downloads/-/gnu-rm)
- The NRF52 SDK 15.3.0 : [nRF-SDK v15.3.0](https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip)
- The Python 3 modules `cbor`, `intelhex`, `click` and `cryptography` modules for the `mcuboot` tool (see [requirements.txt](../tools/mcuboot/requirements.txt))
  - To keep the system clean, you can install python modules into a python virtual environment (`venv`)
    ```sh
    python -m venv .venv
    source .venv/bin/activate
    python -m pip install wheel
    python -m pip install -r tools/mcuboot/requirements.txt
    ```
- A reasonably recent version of CMake (I use 3.16.5)
- lv_font_conv, to generate the font .c files
  - see [lv_font_conv](https://github.com/lvgl/lv_font_conv#install-the-script)
  - install npm (commonly done via the package manager, ensure node's version is at least 12)
  - install lv_font_conv: `npm install lv_font_conv`

## Build steps

### Clone the repo

```
git clone https://github.com/InfiniTimeOrg/InfiniTime.git
cd InfiniTime
git submodule update --init
mkdir build
cd build
```

### Project generation using CMake

CMake configures the project according to variables you specify the command line. The variables are:

 Variable | Description | Example|
----------|-------------|--------|
**ARM_NONE_EABI_TOOLCHAIN_PATH**|path to the toolchain directory|`-DARM_NONE_EABI_TOOLCHAIN_PATH=/home/jf/nrf52/gcc-arm-none-eabi-10.3-2021.10/`|
**NRF5_SDK_PATH**|path to the NRF52 SDK|`-DNRF5_SDK_PATH=/home/jf/nrf52/Pinetime/sdk`|
**CMAKE_BUILD_TYPE (\*)**| Build type (Release or Debug). Release is applied by default if this variable is not specified.|`-DCMAKE_BUILD_TYPE=Debug`
**BUILD_DFU (\*\*)**|Build DFU files while building (needs [adafruit-nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil)).|`-DBUILD_DFU=1`
**BUILD_RESOURCES (\*\*)**| Generate external resource while building (needs [lv_font_conv](https://github.com/lvgl/lv_font_conv) and [lv_img_conv](https://github.com/lvgl/lv_img_conv). |`-DBUILD_RESOURCES=1`
**TARGET_DEVICE**|Target device, used for hardware configuration. Allowed: `PINETIME, MOY-TFK5, MOY-TIN5, MOY-TON5, MOY-UNK`|`-DTARGET_DEVICE=PINETIME` (Default)

#### (\*) Note about **CMAKE_BUILD_TYPE**
By default, this variable is set to *Release*. It compiles the code with size and speed optimizations. We use this value for all the binaries we publish when we [release](https://github.com/InfiniTimeOrg/InfiniTime/releases) new versions of InfiniTime.

The *Debug* mode disables all optimizations, which makes the code easier to debug. However, the binary size will likely be too big to fit in the internal flash memory. If you want to build and debug a *Debug* binary, you can disable some parts of the code that are not needed for the test you want to achieve. You can also apply the *Debug* mode selectively on parts of the application by applying the `DEBUG_FLAGS` only for the part (CMake target) you want to debug. For example, let's say you want to debug code related to LittleFS, simply set the compilation options for the RELEASE configuration of the target to `DEBUG_FLAGS` (in `src/CMakeLists.txt`). This will force the compilation of that target in *Debug* mode while the rest of the project will be built in *Release* mode. Example:

```
target_compile_options(littlefs PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${DEBUG_FLAGS}> # Change from RELEASE_FLAGS to DEBUG_FLAGS
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )
```

#### (\*\*) Note about **BUILD_DFU**
DFU files are the files you'll need to install your build of InfiniTime using OTA (over-the-air) mechanism. To generate the DFU file, the Python tool [adafruit-nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil) is needed on your system. Check that this tool is properly installed before enabling this option.

#### CMake command 

```
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=...
```

### Build the project

During the project generation, CMake created the following targets:

- **pinetime-app** : build the standalone (without bootloader support) version of the firmware.
- **pinetime-recovery** : build the standalone recovery version of infinitime (light firmware that only supports OTA and basic UI)
- **pinetime-recovery-loader** : build the standalone tool that flashes the recovery firmware into the external SPI flash
- **pinetime-mcuboot-app** : build the firmware with the support of the bootloader (based on MCUBoot).
- **pinetime-mcuboot-recovery** : build pinetime-recovery with bootloader support
- **pinetime-mcuboot-recovery-loader** : build pinetime-recovery-loader with bootloader support

If you just want to build the project and run it on the Pinetime, using *pinetime-app* is recommended. See [this page](../bootloader/README.md) for more info about bootloader support.

Build:

```
make -j4 pinetime-app
```

List of files generated:
Binary files are generated into the folder `src`:

- **pinetime-app.bin, .hex and .out** : standalone firmware in bin, hex and out formats.
- **pinetime-app.map** : map file
- **pinetime-mcuboot-app.bin, .hex and .out** : firmware with bootloader support in bin, hex and out formats.
- **pinetime-mcuboot-app.map** : map file
- **pinetime-mcuboot-app-image** : MCUBoot image of the firmware
- **pinetime-mcuboot-app-dfu** : DFU file of the firmware

The same files are generated for **pinetime-recovery** and **pinetime-recoveryloader**
