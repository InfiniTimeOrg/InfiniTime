# PineTime
> The PineTime is a free and open source smartwatch capable of running custom-built open operating systems. Some of the notable features include a heart rate monitor, a week-long battery as well as a capacitive touch IPS display that is legible in direct sunlight. It is a fully community driven side-project, which means that it will ultimately be up to the developers and end-users to determine when they deem the PineTime ready to ship.

> We envision the PineTime as a companion for not only your PinePhone but also for your favorite devices — any phone, tablet, or even PC.

*https://www.pine64.org/pinetime/* 

The goal of this project is to build a firmware designed for the PineTime around FreeRTOS.
It's coded using a mix of C and C++ : the NRF52-SDK and FreeRTOS are written in C, but I prefer to write my own code in C++.

Please **NOTE : for now, this project is in heavy development state**. Feel free to use it, contribute to it or fork it !

This project is based on https://github.com/JF002/nrf52-baseproject.
It's a CMake project that configure everything needed to build applications for the PineTime (https://wiki.pine64.org/index.php/PineTime).

I tested this project (compile only) with the following versions:

  * gcc-arm-none-eabi-8-2019-q3-update (from https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
  * nRF5_SDK_15.3.0_59ac345 (from https://www.nordicsemi.com/Software-and-Tools/Software/nRF5-SDK)
  
I've tested this project on the actual PineTime hardware.

## Current state

 * Project builds and runs on the Pinetime;
 * Logs available via JLink RTT;
 * Basic LCD driver.

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