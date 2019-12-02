# PineTime
This is a CMake project that configure everything needed to build applications for the NRF52 MCU. It configures the toolchain (arm-none-eabi) and the NRF52 SDK.

The CMake files are taken from https://github.com/Polidea/cmake-nRF5x

I tested this project (compile only) with the following versions:

  * gcc-arm-none-eabi-8-2019-q3-update (from https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
  * nRF5_SDK_15.3.0_59ac345 (from https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK)
  
I've tested this project on the NRF52-DK board.

## How to use it

  * Download and unzip arm-none-eabi and NRF52 SDK
  * Clone this repo
  * Call CMake with the following command line argument
  
      - -DARM_NONE_EABI_TOOLCHAIN_PATH=[Path to the toolchain] 
      - -DNRF5_SDK_PATH=[Path to the SDK]
      - -DNRFJPROG=[Path to NRFJProg]
      
```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DARM_NONE_EABI_TOOLCHAIN_PATH=... -DNRF5_SDK_PATH=... -DNRFJPROG=... ../
```

  * Make
```
$ make -j
```  

## RTT

RTT is a feature from Segger's JLink devices that allows bidirectionnal communication between the debugger and the target.
This feature can be used to get the logs from the embedded software on the development computer.

  * Program the MCU with the code (see above)
  * Start JLinkExe
  
```
$ JLinkExe -device nrf52 -if swd -speed 4000 -autoconnect 1
```

  * Start JLinkRTTClient
  
```
$ JLinkRTTClient
```