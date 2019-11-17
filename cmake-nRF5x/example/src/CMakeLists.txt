cmake_minimum_required(VERSION 3.12)

project(BlinkyExample C ASM)

# define some variables just for this example to determine file locations
set(NRF_PROJECT_NAME ble_app_blinky)
set(NRF_BOARD pca10040)
set(NRF_SOFTDEVICE s132)

# define some convenience variables to point to example project directories
set(NRF_PROJECT_PATH ${NRF5_SDK_PATH}/examples/ble_peripheral/${NRF_PROJECT_NAME})
set(NRF_PROJECT_DEVICE_PATH ${NRF_PROJECT_PATH}/${NRF_BOARD}/${NRF_SOFTDEVICE})

# you can specify the location of the linker script if desired instead of using a specific file name
set(NRF5_LINKER_SCRIPT ${NRF_PROJECT_DEVICE_PATH}/armgcc/${NRF_PROJECT_NAME}_gcc_${NRF_TARGET}.ld)

nRF5x_setup()

nRF5x_addAppScheduler()
nRF5x_addAppFIFO()
nRF5x_addAppTimer()
nRF5x_addAppUART()
nRF5x_addAppButton()
nRF5x_addBSP(TRUE FALSE FALSE)
nRF5x_addBLEGATT()

nRF5x_addBLEService(ble_lbs)

add_definitions(-DCONFIG_GPIO_AS_PINRESET)

# usually you would include files in this directory here, like so:
#include_directories(.)
#list(APPEND SOURCE_FILES main.c)

# for example projects we include the example source files
include_directories(${NRF_PROJECT_DEVICE_PATH}/config)
list(APPEND SOURCE_FILES ${NRF_PROJECT_PATH}/main.c)

nRF5x_addExecutable(BlinkyExample "${SOURCE_FILES}")