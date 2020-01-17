cmake_minimum_required(VERSION 3.15)
project(pinetime C CXX ASM)

set(NRF_TARGET "nrf52")

if (NOT ARM_NONE_EABI_TOOLCHAIN_PATH)
  message(FATAL_ERROR "The path to the toolchain (arm-non-eabi) must be specified on the command line (add -DARM_NONE_EABI_TOOLCHAIN_PATH=<path>")
endif ()

if (NOT NRF5_SDK_PATH)
  message(FATAL_ERROR "The path to the NRF52 SDK must be specified on the command line (add -DNRF5_SDK_PATH=<path>")
endif ()

if(NOT USE_JLINK AND NOT USE_GDB_CLIENT)
  set(USE_JLINK true)
endif()

if(USE_JLINK)
  if (NOT NRFJPROG)
    message(FATAL_ERROR "the path to the tool nrfjprog must be specified on the command line (add -DNRFJPROG=<path>")
  endif ()
endif()

if(USE_GDB_CLIENT)
  if(NOT GDB_CLIENT_BIN_PATH)
    set(GDB_CLIENT_BIN_PATH "arm-none-eabi-gdb")
  endif()

  if(NOT GDB_CLIENT_TARGET_REMOTE)
    message(FATAL_ERROR "The GDB target must be specified (add -DGDB_CLIENT_TARGET_REMOTE=<target>")
  endif()
endif()

message("BUILD CONFIGURATION")
message("-------------------")
message("    * Toolchain : " ${ARM_NONE_EABI_TOOLCHAIN_PATH})
message("    * NRF52 SDK : " ${NRF5_SDK_PATH})
set(PROGRAMMER "???")
if(USE_JLINK)
  message("    * Programmer/debugger : JLINK")
  message("    * NrfJprog : " ${NRFJPROG})
elseif(USE_GDB_CLIENT)
  message("    * Programmer/debugger : GDB Client")
  message("    * GDB Client path : " ${GDB_CLIENT_BIN_PATH})
  message("    * GDB Target : " ${GDB_CLIENT_TARGET_REMOTE})
endif()


include("cmake-nRF5x/CMake_nRF5x.cmake")
add_subdirectory(src)
