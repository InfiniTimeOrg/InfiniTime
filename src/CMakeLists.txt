cmake_minimum_required(VERSION 3.12)

project(pinetime-app C CXX ASM)

# define some variables just for this example to determine file locations
set(NRF_PROJECT_NAME pinetime-app)
set(NRF_BOARD pca10040)
#set(NRF_SOFTDEVICE s132)

nRF5x_toolchainSetup()
nRF5x_setup()

#nRF5x_addAppScheduler()
#nRF5x_addAppFIFO()
#nRF5x_addAppTimer()
#nRF5x_addAppUART()
nRF5x_addAppButton()
nRF5x_addBSP(FALSE FALSE FALSE)
nRF5x_addAppGpiote()
#nRF5x_addBLEGATT()
#
#nRF5x_addBLEService(ble_lbs)

add_definitions(-DCONFIG_GPIO_AS_PINRESET)
add_definitions(-DDEBUG)


include_directories(.)

list(APPEND SOURCE_FILES
  Logging/NrfLogger.cpp
  BlinkApp/BlinkApp.cpp
  DisplayApp/DisplayApp.cpp
  DisplayApp/lcdfont70.c
  DisplayApp/lcdfont14.c
  main.cpp
  drivers/St7789.cpp
  drivers/SpiMaster.cpp
  Components/Gfx/Gfx.cpp
  BLE/BleManager.c
  Components/Battery/BatteryController.cpp
  Components/Ble/BleController.cpp
  Components/DateTime/DateTimeController.cpp
  drivers/Cst816s.cpp
  FreeRTOS/port.c
  FreeRTOS/port_cmsis_systick.c
  FreeRTOS/port_cmsis.c
)

set(INCLUDE_FILES
  Logging/Logger.h
  Logging/NrfLogger.h
  BlinkApp/BlinkApp.h
  DisplayApp/DisplayApp.h
  DisplayApp/lcdfont70.h
  DisplayApp/lcdfont14.h
  drivers/St7789.h
  drivers/SpiMaster.h
  Components/Gfx/Gfx.h
  BLE/BleManager.h
  Components/Battery/BatteryController.h
  Components/Ble/BleController.h
  Components/DateTime/DateTimeController.h
  drivers/Cst816s.h
  FreeRTOS/portmacro.h
  FreeRTOS/portmacro_cmsis.h
  libs/date/includes/date/tz.h
  libs/date/includes/date/chrono_io.h
  libs/date/includes/date/date.h
  libs/date/includes/date/islamic.h
  libs/date/includes/date/iso_week.h
  libs/date/includes/date/julian.h
  libs/date/includes/date/ptz.h
  libs/date/includes/date/tz_private.h
)

include_directories(
  FreeRTOS/
  libs/date/includes
)

nRF5x_addExecutable(pinetime-app "${SOURCE_FILES}")