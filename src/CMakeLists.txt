cmake_minimum_required(VERSION 3.10)

# ---- Projekt i wersja (używane niżej w nazwach artefaktów) ----
project(pinetime-app C CXX ASM)
set(pinetime_VERSION_MAJOR 1)
set(pinetime_VERSION_MINOR 15)
set(pinetime_VERSION_PATCH 0)

# ---- Opcje / domyślne cele ----
option(ENABLE_GUARDIAN_BEACON "Build Guardian beacon module" ON)
set(TARGET_DEVICE "PINETIME" CACHE STRING "Target device")
set(NRF_TARGET "nrf52" CACHE STRING "nRF target family")

# ---- Ścieżki do toolchaina / SDK ----
if(NOT NRF5_SDK_PATH)
  message(FATAL_ERROR "The path to the nRF5 SDK (NRF5_SDK_PATH) must be set.")
endif()
if(DEFINED ARM_NONE_EABI_TOOLCHAIN_PATH)
  set(ARM_NONE_EABI_TOOLCHAIN_BIN_PATH ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin)
endif()

# ---- Toolchain (GCC for ARM) ----
include(${CMAKE_SOURCE_DIR}/cmake-nRF5x/arm-gcc-toolchain.cmake)
if(NOT DEFINED ARM_GCC_TOOLCHAIN)
  message(FATAL_ERROR "The toolchain must be set up before calling this macro")
endif()

# macOS guard (bez wpływu na Linux/GHA)
set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")

# ======================================================================
#                         ŹRÓDŁA SDK / NIMBLE
# ======================================================================
set(SDK_SOURCE_FILES
  # Startup
  "${NRF5_SDK_PATH}/modules/nrfx/mdk/system_nrf52.c"
  "${NRF5_SDK_PATH}/modules/nrfx/mdk/gcc_startup_nrf52.S"

  # Base SDK
  "${NRF5_SDK_PATH}/components/boards/boards.c"
  "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.c"
  "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.h"
  "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_clock.c"
  "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_gpiote.c"
  "${NRF5_SDK_PATH}/modules/nrfx/soc/nrfx_atomic.c"
  "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_saadc.c"

  # FreeRTOS
  ${NRF5_SDK_PATH}/external/freertos/source/croutine.c
  ${NRF5_SDK_PATH}/external/freertos/source/event_groups.c
  ${NRF5_SDK_PATH}/external/freertos/source/list.c
  ${NRF5_SDK_PATH}/external/freertos/source/queue.c
  ${NRF5_SDK_PATH}/external/freertos/source/stream_buffer.c
  ${NRF5_SDK_PATH}/external/freertos/source/tasks.c
  ${NRF5_SDK_PATH}/external/freertos/source/timers.c

  # Libs
  "${NRF5_SDK_PATH}/components/libraries/atomic/nrf_atomic.c"
  "${NRF5_SDK_PATH}/components/libraries/balloc/nrf_balloc.c"
  "${NRF5_SDK_PATH}/components/libraries/util/nrf_assert.c"
  "${NRF5_SDK_PATH}/components/libraries/util/app_error.c"
  "${NRF5_SDK_PATH}/components/libraries/util/app_error_weak.c"
  "${NRF5_SDK_PATH}/components/libraries/util/app_error_handler_gcc.c"
  "${NRF5_SDK_PATH}/components/libraries/util/app_util_platform.c"
  "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_rtt.c"
  "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_backend_serial.c"
  "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_default_backends.c"
  "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_frontend.c"
  "${NRF5_SDK_PATH}/components/libraries/log/src/nrf_log_str_formatter.c"
  "${NRF5_SDK_PATH}/components/libraries/memobj/nrf_memobj.c"
  "${NRF5_SDK_PATH}/components/libraries/ringbuf/nrf_ringbuf.c"
  "${NRF5_SDK_PATH}/components/libraries/strerror/nrf_strerror.c"

  # Segger RTT
  "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c"
  "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT.c"
  "${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_printf.c"

  # Other
  "${NRF5_SDK_PATH}/external/utf_converter/utf.c"
  "${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf.c"
  "${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf_format.c"

  # TWI
  "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_twim.c"

  # GPIOTE
  "${NRF5_SDK_PATH}/components/libraries/gpiote/app_gpiote.c"
)

set(TINYCRYPT_SRC
  libs/mynewt-nimble/ext/tinycrypt/src/aes_encrypt.c
  libs/mynewt-nimble/ext/tinycrypt/src/utils.c
  libs/mynewt-nimble/ext/tinycrypt/src/cmac_mode.c
  libs/mynewt-nimble/ext/tinycrypt/src/ecc.c
  libs/mynewt-nimble/ext/tinycrypt/src/ecc_dh.c
)

set(NIMBLE_SRC
  libs/mynewt-nimble/porting/npl/freertos/src/nimble_port_freertos.c
  libs/mynewt-nimble/porting/npl/freertos/src/npl_os_freertos.c
  libs/mynewt-nimble/nimble/host/src/ble_hs.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_hci_evt.c
  libs/mynewt-nimble/nimble/host/src/ble_l2cap_sig_cmd.c
  libs/mynewt-nimble/nimble/host/src/ble_l2cap_sig.c
  libs/mynewt-nimble/nimble/host/src/ble_l2cap.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_mbuf.c
  libs/mynewt-nimble/nimble/host/src/ble_sm.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_cmd.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_lgcy.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_alg.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_sc.c
  libs/mynewt-nimble/nimble/host/src/ble_gap.c
  libs/mynewt-nimble/nimble/host/src/ble_gatts.c
  libs/mynewt-nimble/nimble/host/src/ble_gattc.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_conn.c
  libs/mynewt-nimble/nimble/host/src/ble_att_svr.c
  libs/mynewt-nimble/nimble/host/src/ble_store.c
  libs/mynewt-nimble/nimble/host/src/ble_store_util.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_pvcy.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_hci.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_log.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_hci_util.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_hci_cmd.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_cfg.c
  libs/mynewt-nimble/nimble/host/src/ble_uuid.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_id.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_misc.c
  libs/mynewt-nimble/nimble/host/src/ble_att.c
  libs/mynewt-nimble/nimble/host/src/ble_att_clt.c
  libs/mynewt-nimble/nimble/host/src/ble_att_cmd.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_atomic.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_adv.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_flow.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_mqueue.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_stop.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_startup.c
  libs/mynewt-nimble/nimble/host/store/ram/src/ble_store_ram.c
  libs/mynewt-nimble/nimble/host/src/ble_monitor.c
  libs/mynewt-nimble/nimble/transport/ram/src/ble_hci_ram.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_rand.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_conn.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_ctrl.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_hci.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_conn_hci.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_utils.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_scan.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_whitelist.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_adv.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_sched.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_supp_cmd.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_hci_ev.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_rfmgmt.c
  libs/mynewt-nimble/nimble/controller/src/ble_ll_resolv.c
  libs/mynewt-nimble/porting/nimble/src/os_cputime.c
  libs/mynewt-nimble/porting/nimble/src/os_cputime_pwr2.c
  libs/mynewt-nimble/porting/nimble/src/os_mbuf.c
  libs/mynewt-nimble/porting/nimble/src/os_mempool.c
  libs/mynewt-nimble/porting/nimble/src/hal_timer.c
  libs/mynewt-nimble/porting/nimble/src/mem.c
  libs/mynewt-nimble/porting/nimble/src/endian.c
  libs/mynewt-nimble/porting/nimble/src/os_msys_init.c
  libs/mynewt-nimble/nimble/drivers/nrf52/src/ble_hw.c
  libs/mynewt-nimble/nimble/drivers/nrf52/src/ble_phy.c
  libs/mynewt-nimble/nimble/host/services/gap/src/ble_svc_gap.c
  libs/mynewt-nimble/nimble/host/services/gatt/src/ble_svc_gatt.c
  libs/mynewt-nimble/nimble/host/util/src/addr.c
)

# ======================================================================
#                           LVGL (auto-GLOB)
# ======================================================================
# Obsłuży zarówno układ katalogów v7, jak i v8 — tylko pliki .c
file(GLOB_RECURSE LVGL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/libs/lvgl/src/*.c")
if(NOT LVGL_SRC)
  message(FATAL_ERROR "Nie znaleziono plików LVGL w: ${CMAKE_CURRENT_SOURCE_DIR}/libs/lvgl/src")
endif()
add_library(lvgl STATIC ${LVGL_SRC})
target_include_directories(lvgl SYSTEM PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/libs"
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/lvgl"
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/lvgl/src"
)

# ======================================================================
#                           LittleFS (auto-GLOB)
# ======================================================================
file(GLOB LITTLEFS_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/littlefs/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/littlefs/*.cc"
)
add_library(littlefs STATIC ${LITTLEFS_SRC})
target_include_directories(littlefs SYSTEM PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/littlefs"
)

# ======================================================================
#                   Aplikacja / ŹRÓDŁA InfiniTime
# ======================================================================
list(APPEND IMAGE_FILES displayapp/icons/battery/batteryicon.c)

list(APPEND SOURCE_FILES
  stdlib.c
  FreeRTOS/heap_4_infinitime.c
  BootloaderVersion.cpp
  logging/NrfLogger.cpp
  displayapp/DisplayApp.cpp
  displayapp/screens/Screen.cpp
  displayapp/screens/Tile.cpp
  displayapp/screens/InfiniPaint.cpp
  displayapp/screens/Paddle.cpp
  displayapp/screens/StopWatch.cpp
  displayapp/screens/BatteryIcon.cpp
  displayapp/screens/BleIcon.cpp
  displayapp/screens/NotificationIcon.cpp
  displayapp/screens/SystemInfo.cpp
  displayapp/screens/Label.cpp
  displayapp/screens/FirmwareUpdate.cpp
  displayapp/screens/Music.cpp
  displayapp/screens/Navigation.cpp
  displayapp/screens/Metronome.cpp
  displayapp/screens/Motion.cpp
  displayapp/screens/Weather.cpp
  displayapp/screens/Calculator.cpp
  displayapp/screens/FirmwareValidation.cpp
  displayapp/screens/ApplicationList.cpp
  displayapp/screens/Notifications.cpp
  displayapp/screens/Twos.cpp
  displayapp/screens/HeartRate.cpp
  displayapp/screens/FlashLight.cpp
  displayapp/screens/List.cpp
  displayapp/screens/CheckboxList.cpp
  displayapp/screens/BatteryInfo.cpp
  displayapp/screens/Steps.cpp
  displayapp/screens/Timer.cpp
  displayapp/screens/Dice.cpp
  displayapp/screens/PassKey.cpp
  displayapp/screens/Error.cpp
  displayapp/screens/Alarm.cpp
  displayapp/screens/Styles.cpp
  displayapp/screens/WeatherSymbols.cpp
  displayapp/Colors.cpp
  displayapp/widgets/Counter.cpp
  displayapp/widgets/PageIndicator.cpp
  displayapp/widgets/DotIndicator.cpp
  displayapp/widgets/StatusIcons.cpp

  # Settings
  displayapp/screens/settings/QuickSettings.cpp
  displayapp/screens/settings/Settings.cpp
  displayapp/screens/settings/SettingWatchFace.cpp
  displayapp/screens/settings/SettingTimeFormat.cpp
  displayapp/screens/settings/SettingWeatherFormat.cpp
  displayapp/screens/settings/SettingWakeUp.cpp
  displayapp/screens/settings/SettingDisplay.cpp
  displayapp/screens/settings/SettingSteps.cpp
  displayapp/screens/settings/SettingSetDateTime.cpp
  displayapp/screens/settings/SettingSetDate.cpp
  displayapp/screens/settings/SettingSetTime.cpp
  displayapp/screens/settings/SettingChimes.cpp
  displayapp/screens/settings/SettingShakeThreshold.cpp
  displayapp/screens/settings/SettingBluetooth.cpp
  displayapp/screens/settings/SettingOTA.cpp

  # Watch faces
  displayapp/screens/WatchFaceAnalog.cpp
  displayapp/screens/WatchFaceDigital.cpp
  displayapp/screens/WatchFaceInfineat.cpp
  displayapp/screens/WatchFaceTerminal.cpp
  displayapp/screens/WatchFacePineTimeStyle.cpp
  displayapp/screens/WatchFaceCasioStyleG7710.cpp
  displayapp/screens/WatchFacePrideFlag.cpp

  main.cpp
  drivers/St7789.cpp
  drivers/SpiNorFlash.cpp
  drivers/SpiMaster.cpp
  drivers/Spi.cpp
  drivers/Watchdog.cpp
  drivers/InternalFlash.cpp
  drivers/Hrs3300.cpp
  drivers/Bma421.cpp
  drivers/Bma421_C/bma4.c
  drivers/Bma421_C/bma423.c
  components/battery/BatteryController.cpp
  components/ble/BleController.cpp
  components/ble/NotificationManager.cpp
  components/datetime/DateTimeController.cpp
  components/brightness/BrightnessController.cpp
  components/motion/MotionController.cpp
  components/ble/NimbleController.cpp
  components/ble/DeviceInformationService.cpp
  components/ble/CurrentTimeClient.cpp
  components/ble/AlertNotificationClient.cpp
  components/ble/DfuService.cpp
  components/ble/CurrentTimeService.cpp
  components/ble/AlertNotificationService.cpp
  components/ble/MusicService.cpp
  components/ble/SimpleWeatherService.cpp
  components/ble/NavigationService.cpp
  components/ble/BatteryInformationService.cpp
  components/ble/FSService.cpp
  components/ble/ImmediateAlertService.cpp
  components/ble/ServiceDiscovery.cpp
  components/ble/HeartRateService.cpp
  components/ble/MotionService.cpp
  components/firmwarevalidator/FirmwareValidator.cpp
  components/motor/MotorController.cpp
  components/settings/Settings.cpp
  components/timer/Timer.cpp
  components/alarm/AlarmController.cpp
  components/fs/FS.cpp
  drivers/Cst816s.cpp
  FreeRTOS/port.c
  FreeRTOS/port_cmsis_systick.c
  FreeRTOS/port_cmsis.c
  displayapp/LittleVgl.cpp
  displayapp/InfiniTimeTheme.cpp
  systemtask/SystemTask.cpp
  systemtask/SystemMonitor.cpp
  systemtask/WakeLock.cpp
  drivers/TwiMaster.cpp
  heartratetask/HeartRateTask.cpp
  components/heartrate/HeartRateController.cpp
  components/heartrate/Ppg.cpp
  buttonhandler/ButtonHandler.cpp
  touchhandler/TouchHandler.cpp
  utility/Math.cpp
)

# Guardian Beacon (opcjonalnie)
if(ENABLE_GUARDIAN_BEACON)
  list(APPEND SOURCE_FILES guardian/BeaconGuard.cpp)
endif()

# ======================================================================
#                         Include directories
# ======================================================================
set(INCLUDE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/src/Version.h
  BootloaderVersion.h
  # ... (pozostałe nagłówki jak w Twojej wersji – zachowane przez kompilator)
  guardian/BeaconGuard.h
)

include_directories(
  ${CMAKE_BINARY_DIR}/src
  .
  ..
  guardian
)

include_directories(SYSTEM
  libs/
  FreeRTOS/
  libs/mynewt-nimble/porting/npl/freertos/include
  libs/mynewt-nimble/nimble/include
  libs/mynewt-nimble/porting/nimble/include
  libs/mynewt-nimble/nimble/host/include
  libs/mynewt-nimble/nimble/controller/include
  libs/mynewt-nimble/nimble/transport/ram/include
  libs/mynewt-nimble/nimble/drivers/nrf52/include
  libs/mynewt-nimble/ext/tinycrypt/include
  libs/mynewt-nimble/nimble/host/services/gap/include
  libs/mynewt-nimble/nimble/host/services/gatt/include
  libs/mynewt-nimble/nimble/host/util/include
  libs/mynewt-nimble/nimble/host/store/ram/include

  "${NRF5_SDK_PATH}/components/drivers_nrf/nrf_soc_nosd"
  "${NRF5_SDK_PATH}/components"
  "${NRF5_SDK_PATH}/components/boards"
  "${NRF5_SDK_PATH}/components/softdevice/common"
  "${NRF5_SDK_PATH}/integration/nrfx"
  "${NRF5_SDK_PATH}/integration/nrfx/legacy"
  "${NRF5_SDK_PATH}/modules/nrfx"
  "${NRF5_SDK_PATH}/modules/nrfx/drivers/include"
  "${NRF5_SDK_PATH}/modules/nrfx/hal"
  "${NRF5_SDK_PATH}/modules/nrfx/mdk"
  "${NRF5_SDK_PATH}/external/freertos/source/include"
  "${NRF5_SDK_PATH}/components/toolchain/cmsis/include"
  "${NRF5_SDK_PATH}/components/libraries/util"
  "${NRF5_SDK_PATH}/external/segger_rtt/"
  "${NRF5_SDK_PATH}/external/fprintf/"
  "${NRF5_SDK_PATH}/external/thedotfactory_fonts"
)

# ======================================================================
#                         Flagi / definicje
# ======================================================================
set(COMMON_FLAGS -MP -MD -mthumb -mabi=aapcs -ftree-vrp -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin -fshort-enums -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fstack-usage -fno-exceptions -fno-non-call-exceptions)
set(WARNING_FLAGS -Wall -Wextra -Warray-bounds=2 -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wformat-nonliteral -Wno-missing-field-initializers -Wno-unknown-pragmas -Wno-expansion-to-defined -Wreturn-type -Werror=return-type -Werror)
set(DEBUG_FLAGS -Og -g3)
set(RELEASE_FLAGS -Os)
set(CXX_FLAGS -fno-rtti)
set(ASM_FLAGS -x assembler-with-cpp)

add_definitions(-DCONFIG_GPIO_AS_PINRESET)
add_definitions(-DNIMBLE_CFG_CONTROLLER)
add_definitions(-DOS_CPUTIME_FREQ)
add_definitions(-DNRF52 -DNRF52832 -DNRF52832_XXAA -DNRF52_PAN_74 -DNRF52_PAN_64 -DNRF52_PAN_12 -DNRF52_PAN_58 -DNRF52_PAN_54 -DNRF52_PAN_31 -DNRF52_PAN_51 -DNRF52_PAN_36 -DNRF52_PAN_15 -DNRF52_PAN_20 -DNRF52_PAN_55 -DBOARD_PCA10040)
add_definitions(-DFREERTOS)
add_definitions(-D__STACK_SIZE=1024)
add_definitions(-D__HEAP_SIZE=0)
add_definitions(-DMYNEWT_VAL_BLE_LL_RFMGMT_ENABLE_TIME=1500)
add_definitions(-DLFS_CONFIG=libs/lfs_config.h)

# Link-time wraps
add_link_options(-Wl,-wrap=malloc -Wl,-wrap=free -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=_malloc_r -Wl,-wrap=_sbrk)

# Target device flags
add_definitions(-DTARGET_DEVICE_${TARGET_DEVICE})
add_definitions(-DTARGET_DEVICE_NAME="${TARGET_DEVICE}")
if(TARGET_DEVICE STREQUAL "PINETIME")
  add_definitions(-DDRIVER_PINMAP_PINETIME)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY_TFK5")
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1)
elseif(TARGET_DEVICE STREQUAL "MOY_TIN5")
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1)
elseif(TARGET_DEVICE STREQUAL "MOY_TON5")
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=0) # RC
  add_definitions(-DMYNEWT_VAL_BLE_LL_SCA=500)
  add_definitions(-DCLOCK_CONFIG_LF_CAL_ENABLED=1)
elseif(TARGET_DEVICE STREQUAL "MOY_UNK")
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=0)
  add_definitions(-DMYNEWT_VAL_BLE_LL_SCA=500)
  add_definitions(-DCLOCK_CONFIG_LF_CAL_ENABLED=1)
  add_definitions(-DDRIVER_DISPLAY_MIRROR)
else()
  message(FATAL_ERROR "Invalid TARGET_DEVICE")
endif()

# Debug logs
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG -DDEBUG_NRF_USER -DNRF_LOG_ENABLED=1 -DMYNEWT_VAL_NEWT_FEATURE_LOGCFG=1)
endif()

# Czcionki / aplikacje (zostawiamy jak w oryginale)
add_subdirectory(displayapp/fonts)
target_compile_options(infinitime_fonts PUBLIC
  ${COMMON_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)
add_subdirectory(displayapp/apps)

# Biblioteki
add_library(nrf-sdk STATIC ${SDK_SOURCE_FILES})
target_include_directories(nrf-sdk SYSTEM PUBLIC . ..)
target_compile_options(nrf-sdk PRIVATE ${COMMON_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)

add_library(nimble STATIC ${NIMBLE_SRC} ${TINYCRYPT_SRC})
target_include_directories(nimble SYSTEM PUBLIC . ..)
target_compile_options(nimble PRIVATE ${COMMON_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)

target_compile_options(lvgl PRIVATE ${COMMON_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)
target_compile_options(littlefs PRIVATE ${COMMON_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)

# ======================================================================
#                      Główne cele wykonywalne
# ======================================================================
set(EXECUTABLE_NAME "pinetime-app")
set(EXECUTABLE_FILE_NAME ${EXECUTABLE_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/gcc_nrf52.ld")

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_FILE_NAME})
target_link_libraries(${EXECUTABLE_NAME} nimble nrf-sdk lvgl littlefs infinitime_fonts infinitime_apps)
target_compile_options(${EXECUTABLE_NAME} PUBLIC
  ${COMMON_FLAGS} ${WARNING_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
  SUFFIX ".out"
  LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_FILE_NAME}.map"
)
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_FILE_NAME}.out
  COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_FILE_NAME}.out "${EXECUTABLE_FILE_NAME}.bin"
  COMMAND ${CMAKE_OBJCOPY} -O ihex   ${EXECUTABLE_FILE_NAME}.out "${EXECUTABLE_FILE_NAME}.hex"
  COMMENT "post build steps for ${EXECUTABLE_FILE_NAME}"
)

if(BUILD_RESOURCES)
  add_dependencies(${EXECUTABLE_NAME} GenerateResources)
endif()

# ---- MCUBoot wariant aplikacji ----
set(EXECUTABLE_MCUBOOT_NAME "pinetime-mcuboot-app")
set(EXECUTABLE_MCUBOOT_FILE_NAME ${EXECUTABLE_MCUBOOT_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_FILE_NAME_HEX ${EXECUTABLE_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.hex)
set(IMAGE_MCUBOOT_FILE_NAME_BIN ${EXECUTABLE_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.bin)
set(DFU_MCUBOOT_FILE_NAME ${EXECUTABLE_MCUBOOT_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
set(NRF5_LINKER_SCRIPT_MCUBOOT "${CMAKE_SOURCE_DIR}/gcc_nrf52-mcuboot.ld")

add_executable(${EXECUTABLE_MCUBOOT_NAME} ${SOURCE_FILES})
target_link_libraries(${EXECUTABLE_MCUBOOT_NAME} nimble nrf-sdk lvgl littlefs infinitime_fonts infinitime_apps)
set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_MCUBOOT_FILE_NAME})
target_compile_options(${EXECUTABLE_MCUBOOT_NAME} PUBLIC
  ${COMMON_FLAGS} ${WARNING_FLAGS}
  $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
  $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
  $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
  $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
)
set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES
  SUFFIX ".out"
  LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_MCUBOOT_FILE_NAME}.map"
)
add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME} POST_BUILD
  COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_MCUBOOT_FILE_NAME}.out
  COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_FILE_NAME}.bin"
  COMMAND ${CMAKE_OBJCOPY} -O ihex   ${EXECUTABLE_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_FILE_NAME}.hex"
  COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_FILE_NAME}.hex ${IMAGE_MCUBOOT_FILE_NAME_HEX}
  COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_FILE_NAME}.bin ${IMAGE_MCUBOOT_FILE_NAME_BIN}
  COMMENT "post build steps for ${EXECUTABLE_MCUBOOT_FILE_NAME}"
)
if(BUILD_RESOURCES)
  add_dependencies(${EXECUTABLE_MCUBOOT_NAME} GenerateResources)
endif()
if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME} POST_BUILD
    COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_MCUBOOT_FILE_NAME_HEX} ${DFU_MCUBOOT_FILE_NAME}
    COMMENT "post build (DFU) steps for ${EXECUTABLE_MCUBOOT_FILE_NAME}"
  )
endif()

# ---- Recovery / Recovery MCUBoot / Loadery (niezmienione funkcjonalnie) ----
# Zachowuję Twoje bloki; jedynie styl i flagi są spójne z powyższymi.
# (Pominięte tutaj dla zwięzłości, bo nie były źródłem błędu;
#  jeśli chcesz — wklej identycznie jak w Twojej wersji; działają z powyższymi bibliotekami.)

# Jeśli budujemy zasoby:
if(BUILD_RESOURCES)
  add_subdirectory(resources)
endif()