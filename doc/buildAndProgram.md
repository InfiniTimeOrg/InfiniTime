# Build
## Dependencies
To build this project, you'll need:
 - A cross-compiler : [ARM-GCC (arm-none-eabi 11.2-2022.02)](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads)
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
**ARM_NONE_EABI_TOOLCHAIN_PATH**|path to the toolchain directory|`-DARM_NONE_EABI_TOOLCHAIN_PATH=/home/jf/nrf52/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/`|
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

### Using JLink
Start JLinkExe:
```
$ /opt/SEGGER/JLink/JLinkExe -device nrf52 -if swd -speed 4000 -autoconnect 1     
SEGGER J-Link Commander V6.70d (Compiled Apr 16 2020 17:59:37)
DLL version V6.70d, compiled Apr 16 2020 17:59:25

Connecting to J-Link via USB...O.K.
Firmware: J-Link OB-SAM3U128-V2-NordicSemi compiled Mar 17 2020 14:43:00
Hardware version: V1.00
S/N: 682579153
License(s): RDI, FlashBP, FlashDL, JFlash, GDB
VTref=3.300V
Device "NRF52" selected.


Connecting to target via SWD
InitTarget() start
InitTarget() end
Found SW-DP with ID 0x2BA01477
DPIDR: 0x2BA01477
Scanning AP map to find all available APs
AP[2]: Stopped AP scan as end of AP map has been reached
AP[0]: AHB-AP (IDR: 0x24770011)
AP[1]: JTAG-AP (IDR: 0x02880000)
Iterating through AP map to find AHB-AP to use
AP[0]: Core found
AP[0]: AHB-AP ROM base: 0xE00FF000
CPUID register: 0x410FC241. Implementer code: 0x41 (ARM)
Found Cortex-M4 r0p1, Little endian.
FPUnit: 6 code (BP) slots and 2 literal slots
CoreSight components:
ROMTbl[0] @ E00FF000
ROMTbl[0][0]: E000E000, CID: B105E00D, PID: 000BB00C SCS-M7
ROMTbl[0][1]: E0001000, CID: B105E00D, PID: 003BB002 DWT
ROMTbl[0][2]: E0002000, CID: B105E00D, PID: 002BB003 FPB
ROMTbl[0][3]: E0000000, CID: B105E00D, PID: 003BB001 ITM
ROMTbl[0][4]: E0040000, CID: B105900D, PID: 000BB9A1 TPIU
ROMTbl[0][5]: E0041000, CID: B105900D, PID: 000BB925 ETM
Cortex-M4 identified.
J-Link>
```

Use the command loadfile to program the .hex file:
```
J-Link>loadfile pinetime-app.hex 
Downloading file [pinetime-app.hex]...
Comparing flash   [100%] Done.
Erasing flash     [100%] Done.
Programming flash [100%] Done.
Verifying flash   [100%] Done.
J-Link: Flash download: Bank 0 @ 0x00000000: 1 range affected (4096 bytes)
J-Link: Flash download: Total time needed: 0.322s (Prepare: 0.043s, Compare: 0.202s, Erase: 0.003s, Program: 0.064s, Verify: 0.000s, Restore: 0.007s)
O.K.
```

Then reset (r) and start (g) the CPU:
```
J-Link>r
Reset delay: 0 ms
Reset type NORMAL: Resets core & peripherals via SYSRESETREQ & VECTRESET bit.
Reset: Halt core after reset via DEMCR.VC_CORERESET.
Reset: Reset device via AIRCR.SYSRESETREQ.
J-Link>g
```

#### JLink RTT
RTT is a feature from Segger's JLink devices that allows bidirectional communication between the debugger and the target. This feature can be used to get the logs from the embedded software on the development computer.

 - Program the MCU with the code (see above)
 - Start JLinkExe
 
```
$ JLinkExe -device nrf52 -if swd -speed 4000 -autoconnect 1
```

Start JLinkRTTClient
```
$ JLinkRTTClient
```

### Using GDB and Black Magic Probe (BMP)
Enter the following command into GDB:

```
target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
file ./pinetime-app-full.hex 
load
run
```

Example :
```
$ /home/jf/nrf52/gcc-arm-none-eabi-8-2019-q3-update/bin/arm-none-eabi-gdb

(gdb) target extended-remote /dev/ttyACM0
Remote debugging using /dev/ttyACM0
(gdb) monitor swdp_scan
Target voltage: ABSENT!
Available Targets:
No. Att Driver
 1      Nordic nRF52 M3/M4
 2      Nordic nRF52 Access Port 

(gdb) attach 1
Attaching to Remote target
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0xfffffffe in ?? ()
(gdb) file ./pinetime-app-full.hex 
A program is being debugged already.
Are you sure you want to change the file? (y or n) y
Reading symbols from ./pinetime-app-full.hex...
(No debugging symbols found in ./pinetime-app-full.hex)
(gdb) load
Loading section .sec1, size 0xb00 lma 0x0
Loading section .sec2, size 0xf000 lma 0x1000
Loading section .sec3, size 0x10000 lma 0x10000
Loading section .sec4, size 0x5150 lma 0x20000
Loading section .sec5, size 0xa000 lma 0x26000
Loading section .sec6, size 0x10000 lma 0x30000
Loading section .sec7, size 0xdf08 lma 0x40000
Start address 0x0, load size 314200
Transfer rate: 45 KB/sec, 969 bytes/write.
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
