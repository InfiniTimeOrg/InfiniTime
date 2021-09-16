# cmake-nRF5x

Cmake script for projects targeting Nordic Semiconductor nRF5x series devices using the GCC toolchain from ARM.

# Dependencies

The script makes use of the following tools:

- nRF5x SDK by Nordic Semiconductor - SoC specific drivers and libraries (also includes a lot of examples)
- JLink by Segger - interface software for the JLink familiy of programmers
- nrfjprog by Nordic Semiconductor - Wrapper utility around JLink
- arm-non-eabi-gcc by ARM and the GCC Team - compiler toolchain for embedded (= bare metal) ARM chips

# Setup

1. Download this repo (or add as submodule) to the directory `cmake-nRF5x` in your project

1. Search the SDK `example` directory for a `sdk_config.h`, `main.c` and a linker script (normally named `<project_name>_gcc_<chip familly>.ld`) that fits your chip and project needs.

1. Copy the `sdk_config.h` and the project `main.c` into a new directory `src`. Modify them as required for your project.

1. Copy the linker script into the root of your project. Rename it to just `gcc_<chip familly>.ld` For example:
	
	```
	gcc_nrf51.ld
	gcc_nrf52.ld
	```

1. Create a new `CMakeLists.txt` file at the same level. Add the project standard cmake project header

	A typical file may look like this:

	```
	cmake_minimum_required(VERSION 3.6)

	set(NRF_TARGET "nrf52")
	
	# optional, won't be used if passing toolchain on command line (see below)
	if (NOT DEFINED ARM_NONE_EABI_TOOLCHAIN_PATH)
		set(ARM_NONE_EABI_TOOLCHAIN_PATH "/usr/local/bin")
	endif ()
	
	set(NRF5_SDK_PATH "${CMAKE_SOURCE_DIR}/toolchains/nRF5/nRF5_SDK")
	set(NRFJPROG "${CMAKE_SOURCE_DIR}/toolchains/nRF5/nrfjprog/nrfjprog")

	include("cmake-nRF5x/CMake_nRF5x.cmake")

	# must be called before first project call or add_subdirectory unless passing on command line
	nRF5x_toolchainSetup()

	project(YourProjectName C ASM)
	
	nRF5x_setup()

	nRF5x_addAppScheduler()
	nRF5x_addAppFIFO()
	nRF5x_addAppTimer()
	nRF5x_addAppUART()
	nRF5x_addAppButton()
	nRF5x_addBSP(TRUE FALSE FALSE)
	nRF5x_addBLEGATT()


	nRF5x_addBLEService(ble_bas)

	add_definitions(-DCONFIG_GPIO_AS_PINRESET)
			
	include_directories("./src")
	list(APPEND SOURCE_FILES "./src/main.c")

	nRF5x_addExecutable(${PROJECT_NAME} "${SOURCE_FILES}")
	```

	Adjust as needed for your project.

	_Note_: you can add `CXX` between `C ASM` to add c++ support
	
1. Optionally add additional libraries:

	Only the most common drivers and libraries are wrapped with cmake macros.

	To include BLE services, use `nRF5x_addBLEService(<service name>)`.

	For other SDK libraries you can use `include_directories` and `list(APPEND SDK_SOURCE_FILES ...)` to add them. For example:

	```cmake
	include_directories(
	        "${NRF5_SDK_PATH}/<library header directory path>"
	)
		
	list(APPEND SDK_SOURCE_FILES
	        "${NRF5_SDK_PATH}/<library source file path>"
	)
	```
	

# Build

After setup you can use cmake as usual:

1. Generate the actual build files (out-of-source builds are strongly recomended):

	```commandline
	cmake -H. -B"cmake-build" -G "Unix Makefiles"
	```
	You can optionally pass the toolchain to `cmake` when configuring:
    ```
    -DCMAKE_TOOLCHAIN_PATH=cmake-nRF5x/arm-gcc-toolchain.cmake
    ```
    but if you do so you must ensure the toolchain binaries are available in your environment PATH (i.e. work on the command line without specifying absolute path)

2. Build your app:

	```commandline
	cmake --build "cmake-build" --target <your project name>
	```

# Flash

In addition to the build target (named like your project) the script adds some support targets:

`FLASH_SOFTDEVICE` To flash a nRF softdevice to the SoC (typically done only once for each SoC)

```commandline
cmake --build "cmake-build" --target FLASH_SOFTDEVICE
```

`FLASH_<your project name>` To flash your application (this will also rebuild your App first)

```commandline
cmake --build "cmake-build" --target FLASH_<your project name>
```

`FLASH_ERASE` To completely erase the SoC flash

```commandline
cmake --build "cmake-build" --target FLASH_ERASE
```

# JLink Applications

To start the gdb server and RTT terminal, build the target `START_JLINK`:

```commandline
cmake --build "cmake-build" --target START_JLINK
```

# License

MIT for the `CMake_nRF5x.cmake` file. 

Please note that the nRF5x SDK by Nordic Semiconductor is covered by it's own license and shouldn't be re-distributed. 
