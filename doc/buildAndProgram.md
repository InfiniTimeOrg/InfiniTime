# Build
##Dependencies
To build this project, you'll need:
 - A cross-compiler : [gcc-arm-none-eabi-8-2019-q3-update](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads/8-2019q3-update)
 - The NRF52 SDK 15.3.0 : [nRF5_SDK_15.3.0_59ac345](https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip)
 - A reasonably recent version of CMake (I use 3.16.5)

##Build steps 
###Clone the repo
```
git clone https://github.com/JF002/Pinetime.git
cd Pinetime
mkdir build
cd build
```
###Project generation using CMake
CMake configures the project according to variables you specify the command line. The variables are:

 Variable | Description | Example|
----------|-------------|--------|
**ARM_NONE_EABI_TOOLCHAIN_PATH**|path to the toolchain directory|`-DARM_NONE_EABI_TOOLCHAIN_PATH=/home/jf/nrf52/gcc-arm-none-eabi-9-2019-q4-major/`|
**NRF5_SDK_PATH**|path to the NRF52 SDK|`-DNRF5_SDK_PATH=/home/jf/nrf52/Pinetime/sdk`|
**USE_JLINK, USE_GDB_CLIENT and USE_OPENOCD**|Enable *JLink* mode, *GDB Client* (Black Magic Probe) mode or *OpenOCD* mode (set the one you want to use to `1`)|`-DUSE_JLINK=1`
**CMAKE_BUILD_TYPE**| Build type (Release or Debug). Release is applied by default if this variable is not specified.|`-DCMAKE_BUILD_TYPE=Debug`
**NRFJPROG**|Path to the NRFJProg executable. Used only if `USE_JLINK` is 1.|`-DNRFJPROG=/opt/nrfjprog/nrfjprog`
**GDB_CLIENT_BIN_PATH**|Path to arm-none-eabi-gdb executable. Used only if `USE_GDB_CLIENT` is 1.|`-DGDB_CLIENT_BIN_PATH=/home/jf/nrf52/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gdb`
**GDB_CLIENT_TARGET_REMOTE**|Target remote connection string. Used only if `USE_GDB_CLIENT` is 1.|`-DGDB_CLIENT_TARGET_REMOTE=/dev/ttyACM0`


####CMake command line for JLink
```
cmake -DCMAKE_BUILD_TYPE=Debug -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DUSE_JLINK=1 -DNRFJPROG=... ../
```

####CMake command line for GDB Client (Black Magic Probe)
```
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DUSE_GDB_CLIENT=1 -DGDB_CLIENT_BIN_PATH=... -DGDB_CLIENT_TARGET_REMOTE=... ../
```

####CMake command line for OpenOCD
```
cmake -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DUSE_OPENOCD=1 -DGDB_CLIENT_BIN_PATH=[optional] ../
```

###Build the project
During the project generation, CMake created the following targets:
- FLASH_ERASE : mass erase the flash memory of the NRF52.
- FLASH_pinetime-app : flash the firmware into the NRF52.
- pinetime-app : build the standalone (without bootloader support) version of the firmware.
- pinetime-mcuboot-app : build the firmware with the support of the bootloader (based on MCUBoot).
- pinetime-graphics : small firmware that writes the boot graphics into the SPI flash.

If you just want to build the project and run it on the Pinetime, using *pinetime-app* is recommanded. See ???? for more info about bootloader support.

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
 - **pinetime-graphics.bin, .hex and .out** : firmware for the boot graphic in bin, hex and out formats.
 - **pinetime-graphics.map** : map file
 
###Program and run
####Using CMake targets
These target have been configured during the project generation by CMake according to the parameters you provided to the command line.

Mass erase:
```
make FLASH_ERASE
``` 

Flash the application:
```
make FLASH_pinetime-app
```

###Using JLink
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

####JLink RTT
RTT is a feature from Segger's JLink devices that allows bidirectionnal communication between the debugger and the target. This feature can be used to get the logs from the embedded software on the development computer.

 - Program the MCU with the code (see above)
 - Start JLinkExe
 
```
$ JLinkExe -device nrf52 -if swd -speed 4000 -autoconnect 1
```

Start JLinkRTTClient
```
$ JLinkRTTClient
```

###Using GDB and Black Magic Probe (BMP)
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