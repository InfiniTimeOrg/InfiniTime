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
**USE_JLINK, USE_GDB_CLIENT and USE_OPENOCD**|Enable *JLink* mode, *GDB Client* (Black Magic Probe) mode or *OpenOCD* mode (set the one you want to use to `1`)|`-DUSE_JLINK=1`
**CMAKE_BUILD_TYPE (\*)**| Build type (Release or Debug). Release is applied by default if this variable is not specified.|`-DCMAKE_BUILD_TYPE=Debug`
**NRFJPROG**|Path to the NRFJProg executable. Used only if `USE_JLINK` is 1.|`-DNRFJPROG=/opt/nrfjprog/nrfjprog`
**GDB_CLIENT_BIN_PATH**|Path to arm-none-eabi-gdb executable. Used only if `USE_GDB_CLIENT` is 1.|`-DGDB_CLIENT_BIN_PATH=/home/jf/nrf52/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gdb`
**GDB_CLIENT_TARGET_REMOTE**|Target remote connection string. Used only if `USE_GDB_CLIENT` is 1.|`-DGDB_CLIENT_TARGET_REMOTE=/dev/ttyACM0`
**BUILD_DFU (\*\*)**|Build DFU files while building (needs [adafruit-nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil)).|`-DBUILD_DFU=1`
**TARGET_DEVICE**|Target device, used for hardware configuration. Allowed: `PINETIME, MOY-TFK5, MOY-TIN5, MOY-TON5, MOY-UNK`|`-DTARGET_DEVICE=PINETIME` (Default)

####(**) Note about **CMAKE_BUILD_TYPE**:
By default, this variable is set to *Release*. It compiles the code with size and speed optimizations. We use this value for all the binaries we publish when we [release](https://github.com/InfiniTimeOrg/InfiniTime/releases) new versions of InfiniTime.

The *Debug* mode disables all optimizations, which makes the code easier to debug. However, the binary size will likely be too big to fit in the internal flash memory. If you want to build and debug a *Debug* binary, you'll need to disable some parts of the code. For example, the icons for the **Navigation** app use a lot of memory space. You can comment the content of `m_iconMap` in the [Navigation](https://github.com/InfiniTimeOrg/InfiniTime/blob/develop/src/displayapp/screens/Navigation.h#L148) application to free some memory.

####(**) Note about **BUILD_DFU**:
DFU files are the files you'll need to install your build of InfiniTime using OTA (over-the-air) mechanism. To generate the DFU file, the Python tool [adafruit-nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil) is needed on your system. Check that this tool is properly installed before enabling this option.

#### CMake command line for JLink
```
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DUSE_JLINK=1 -DNRFJPROG=... ../
```

#### CMake command line for GDB Client (Black Magic Probe)
```
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DUSE_GDB_CLIENT=1 -DGDB_CLIENT_BIN_PATH=... -DGDB_CLIENT_TARGET_REMOTE=... ../
```

#### CMake command line for OpenOCD
```
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DUSE_OPENOCD=1 -DGDB_CLIENT_BIN_PATH=[optional] ../
```

### Build the project
During the project generation, CMake created the following targets:
- **FLASH_ERASE** : mass erase the flash memory of the NRF52.
- **FLASH_pinetime-app** : flash the firmware into the NRF52.
- **pinetime-app** : build the standalone (without bootloader support) version of the firmware.
- **pinetime-recovery** : build the standalone recovery version of infinitime (light firmware that only supports OTA and basic UI)
- **pinetime-recovery-loader** : build the standalone tool that flashes the recovery firmware into the external SPI flash
- **pinetime-mcuboot-app** : build the firmware with the support of the bootloader (based on MCUBoot).
- **pinetime-mcuboot-recovery** : build pinetime-recovery with bootloader support
- **pinetime-mcuboot-recovery-loader** : build pinetime-recovery-loader with bootloader support

If you just want to build the project and run it on the Pinetime, using *pinetime-app* is recommended. See [this page](../bootloader/README.md) for more info about bootloader support.

Build:
```
make -j pinetime-app
```

List of files generated:
Binary files are generated into the folder `src`:
 - **pinetime-app.bin, .hex and .out** : standalone firmware in bin, hex and out formats.
 - **pinetime-app.map** : map file
 - **pinetime-mcuboot-app.bin, .hex and .out** : firmware with bootloader support in bin, hex and out formats.
 - **pinetime-mcuboot-app.map** : map file
 - **pinetime-mcuboot-app-image** : MCUBoot image of the firmware
 - **pinetime-mcuboot-ap-dfu** : DFU file of the firmware

The same files are generated for **pinetime-recovery** and **pinetime-recoveryloader** 

 
### Program and run

#### Using CMake targets
These target have been configured during the project generation by CMake according to the parameters you provided to the command line.

Mass erase:
```
make FLASH_ERASE
``` 

Flash the application:
```
make FLASH_pinetime-app
```

### How to generate files needed by the factory
These files are needed by the Pine64 factory to flash InfiniTime as the default firmware on the PineTimes.

Two files are needed: an **HEX (.hex)** file that contains the content of the internal flash memory (bootloader + InfiniTime) and a **binary (.bin)** file that contains the content of the external flash memory (recovery firmware).

#### merged-internal.hex
First, convert the bootloader to hex:
 ```
 <ARM TOOLCHAIN>/bin/arm-none-eabi-objcopy -I binary -O ihex ./bootloader.bin ./bootloader.hex
 ```
where `bootloader.bin` is the [last stable version](https://github.com/JF002/pinetime-mcuboot-bootloader/releases) of the [bootloader](https://github.com/JF002/pinetime-mcuboot-bootloader).

Then, convert the MCUBoot image of InfiniTime:
```
<ARM TOOLCHAIN>/bin/arm-none-eabi-objcopy -I binary -O ihex --change-addresses 0x8000 ./pinetime-mcuboot-app-image-1.6.0.bin ./pinetime-mcuboot-app-image-1.6.0.hex
```
where `pinetime-mcuboot-app-image-1.6.0.bin` is [the bin of the last MCUBoot image](https://github.com/InfiniTimeOrg/InfiniTime/releases) of [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime).

Pay attention to the parameter `--change-addresses 0x8000`. It's needed to ensure the image will be flashed at the offset expected by the bootloader (0x8000).

Finally, merge them together with **mergehex**:
```
/opt/mergehex/mergehex -m ./bootloader.hex ./pinetime-mcuboot-app-image-1.6.0.hex  -o merged-internal.hex
```

This file must be flashed at offset **0x00** of the internal memory of the NRF52832.

#### spinor.bin
This file is the MCUBoot image of the last stable version of the recovery firmware. It must be flashed at offset **0x00** of the external SPINOR flash memory.
