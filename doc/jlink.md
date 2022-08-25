# Flashing the firmware with JLink

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

# JLink RTT

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
