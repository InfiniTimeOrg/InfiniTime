cmake_minimum_required(VERSION 3.15)
project(pinetime C CXX ASM)

set(NRF_TARGET "nrf52")

if (NOT ARM_NONE_EABI_TOOLCHAIN_PATH)
  message(FATAL_ERROR "The path to the toolchain (arm-non-eabi) must be specified on the command line (add -DARM_NONE_EABI_TOOLCHAIN_PATH=<path>")
endif ()

if (NOT NRF5_SDK_PATH)
  message(FATAL_ERROR "The path to the NRF52 SDK must be specified on the command line (add -DNRF52_SDK_PATH=<path>")
endif ()

if (NOT NRFJPROG)
  message(FATAL_ERROR "the path to the tool nrfjprog must be specified on the command line (add -DNRFJPROG=<path>")
endif ()

include("cmake-nRF5x/CMake_nRF5x.cmake")
add_subdirectory(src)
