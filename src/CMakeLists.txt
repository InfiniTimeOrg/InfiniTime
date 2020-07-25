cmake_minimum_required(VERSION 3.10)

project(pinetime-app C CXX ASM)

# define some variables just for this example to determine file locations
set(NRF_PROJECT_NAME pinetime-app)
set(NRF_BOARD pca10040)

# check if all the necessary tools paths have been provided.
if (NOT NRF5_SDK_PATH)
    message(FATAL_ERROR "The path to the nRF5 SDK (NRF5_SDK_PATH) must be set.")
endif ()
if(DEFINED ARM_NONE_EABI_TOOLCHAIN_PATH)
    set(ARM_NONE_EABI_TOOLCHAIN_BIN_PATH ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin)
endif()

if (NOT NRF_TARGET MATCHES "nrf52")
    message(FATAL_ERROR "Only rRF52 boards are supported right now")
endif()

# Setup toolchain
include(${CMAKE_SOURCE_DIR}/cmake-nRF5x/arm-gcc-toolchain.cmake)

if(NOT DEFINED ARM_GCC_TOOLCHAIN)
    message(FATAL_ERROR "The toolchain must be set up before calling this macro")
endif()
set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")


set(SDK_SOURCE_FILES
        # Startup
        "${NRF5_SDK_PATH}/modules/nrfx/mdk/system_nrf52.c"
        "${NRF5_SDK_PATH}/modules/nrfx/mdk/gcc_startup_nrf52.S"

        # Base SDK
        "${NRF5_SDK_PATH}/components/boards/boards.c"
        "${NRF5_SDK_PATH}/integration/nrfx/legacy/nrf_drv_clock.c"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_clock.c"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_gpiote.c"
        "${NRF5_SDK_PATH}/modules/nrfx/soc/nrfx_atomic.c"
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_saadc.c"

        # FreeRTOS
        ${NRF5_SDK_PATH}/external/freertos/source/croutine.c
        ${NRF5_SDK_PATH}/external/freertos/source/event_groups.c
        ${NRF5_SDK_PATH}/external/freertos/source/portable/MemMang/heap_4.c
        ${NRF5_SDK_PATH}/external/freertos/source/list.c
        ${NRF5_SDK_PATH}/external/freertos/source/queue.c
        ${NRF5_SDK_PATH}/external/freertos/source/stream_buffer.c
        ${NRF5_SDK_PATH}/external/freertos/source/tasks.c
        ${NRF5_SDK_PATH}/external/freertos/source/timers.c
        ${NRF5_SDK_PATH}/components/libraries/timer/app_timer_freertos.c

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
        "${NRF5_SDK_PATH}/modules/nrfx/drivers/src/nrfx_twi.c"

        # GPIOTE
        "${NRF5_SDK_PATH}/components/libraries/gpiote/app_gpiote.c"
)

set(TINYCRYPT_SRC
  libs/mynewt-nimble/ext/tinycrypt/src/aes_encrypt.c
  libs/mynewt-nimble/ext/tinycrypt/src/utils.c
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
  libs/mynewt-nimble/nimble/host/src/ble_att_svr.c
  libs/mynewt-nimble/nimble/host/src/ble_att_cmd.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_atomic.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_adv.c
  libs/mynewt-nimble/nimble/host/src/ble_hs_flow.c
  libs/mynewt-nimble/nimble/host/src/ble_sm.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_cmd.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_lgcy.c
  libs/mynewt-nimble/nimble/host/src/ble_sm_alg.c
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

set(LVGL_SRC
  libs/lv_conf.h
  libs/lvgl/lvgl.h
  libs/lvgl/src/lv_core/lv_obj.c
  libs/lvgl/src/lv_core/lv_obj.h
  libs/lvgl/src/lv_core/lv_group.c
  libs/lvgl/src/lv_core/lv_group.h
  libs/lvgl/src/lv_core/lv_disp.c
  libs/lvgl/src/lv_core/lv_disp.h
  libs/lvgl/src/lv_core/lv_debug.h
  libs/lvgl/src/lv_core/lv_debug.c
  libs/lvgl/src/lv_core/lv_indev.c
  libs/lvgl/src/lv_core/lv_indev.h
  libs/lvgl/src/lv_core/lv_refr.c
  libs/lvgl/src/lv_core/lv_refr.h
  libs/lvgl/src/lv_core/lv_style.c
  libs/lvgl/src/lv_core/lv_style.h
  libs/lvgl/src/lv_misc/lv_anim.c
  libs/lvgl/src/lv_misc/lv_anim.h
  libs/lvgl/src/lv_misc/lv_async.h
  libs/lvgl/src/lv_misc/lv_async.c
  libs/lvgl/src/lv_misc/lv_fs.c
  libs/lvgl/src/lv_misc/lv_fs.h
  libs/lvgl/src/lv_misc/lv_task.c
  libs/lvgl/src/lv_misc/lv_task.h
  libs/lvgl/src/lv_misc/lv_area.c
  libs/lvgl/src/lv_misc/lv_area.h
  libs/lvgl/src/lv_misc/lv_bidi.c
  libs/lvgl/src/lv_misc/lv_bidi.h
  libs/lvgl/src/lv_misc/lv_circ.c
  libs/lvgl/src/lv_misc/lv_circ.h
  libs/lvgl/src/lv_misc/lv_color.c
  libs/lvgl/src/lv_misc/lv_color.h
  libs/lvgl/src/lv_misc/lv_fs.c
  libs/lvgl/src/lv_misc/lv_fs.h
  libs/lvgl/src/lv_misc/lv_gc.c
  libs/lvgl/src/lv_misc/lv_gc.h
  libs/lvgl/src/lv_misc/lv_ll.c
  libs/lvgl/src/lv_misc/lv_ll.h
  libs/lvgl/src/lv_misc/lv_log.c
  libs/lvgl/src/lv_misc/lv_log.h
  libs/lvgl/src/lv_misc/lv_math.c
  libs/lvgl/src/lv_misc/lv_math.h
  libs/lvgl/src/lv_misc/lv_mem.c
  libs/lvgl/src/lv_misc/lv_mem.h
  libs/lvgl/src/lv_misc/lv_printf.c
  libs/lvgl/src/lv_misc/lv_printf.h
  libs/lvgl/src/lv_misc/lv_task.c
  libs/lvgl/src/lv_misc/lv_task.h
  libs/lvgl/src/lv_misc/lv_templ.c
  libs/lvgl/src/lv_misc/lv_templ.h
  libs/lvgl/src/lv_misc/lv_txt.c
  libs/lvgl/src/lv_misc/lv_txt.h
  libs/lvgl/src/lv_misc/lv_types.h
  libs/lvgl/src/lv_misc/lv_utils.c
  libs/lvgl/src/lv_misc/lv_utils.h
  libs/lvgl/src/lv_draw/lv_draw.c
  libs/lvgl/src/lv_draw/lv_draw.h
  libs/lvgl/src/lv_draw/lv_draw_arc.c
  libs/lvgl/src/lv_draw/lv_draw_arc.h
  libs/lvgl/src/lv_draw/lv_draw_basic.c
  libs/lvgl/src/lv_draw/lv_draw_basic.h
  libs/lvgl/src/lv_draw/lv_draw_img.c
  libs/lvgl/src/lv_draw/lv_draw_img.h
  libs/lvgl/src/lv_draw/lv_draw_label.c
  libs/lvgl/src/lv_draw/lv_draw_label.h
  libs/lvgl/src/lv_draw/lv_draw_line.c
  libs/lvgl/src/lv_draw/lv_draw_line.h
  libs/lvgl/src/lv_draw/lv_draw_rect.c
  libs/lvgl/src/lv_draw/lv_draw_rect.h
  libs/lvgl/src/lv_draw/lv_draw_triangle.c
  libs/lvgl/src/lv_draw/lv_draw_triangle.h
  libs/lvgl/src/lv_draw/lv_img_cache.c
  libs/lvgl/src/lv_draw/lv_img_cache.h
  libs/lvgl/src/lv_draw/lv_img_decoder.c
  libs/lvgl/src/lv_draw/lv_img_decoder.h
  libs/lvgl/src/lv_hal/lv_hal.h
  libs/lvgl/src/lv_hal/lv_hal_disp.c
  libs/lvgl/src/lv_hal/lv_hal_disp.h
  libs/lvgl/src/lv_hal/lv_hal_indev.c
  libs/lvgl/src/lv_hal/lv_hal_indev.h
  libs/lvgl/src/lv_hal/lv_hal_tick.c
  libs/lvgl/src/lv_hal/lv_hal_tick.h
  libs/lvgl/src/lv_font/lv_font.c
  libs/lvgl/src/lv_font/lv_font.h
  libs/lvgl/src/lv_font/lv_font_fmt_txt.c
  libs/lvgl/src/lv_font/lv_font_fmt_txt.h
  libs/lvgl/src/lv_font/lv_symbol_def.h
  libs/lvgl/src/lv_themes/lv_theme.c
  libs/lvgl/src/lv_themes/lv_theme.h
  libs/lvgl/src/lv_objx/lv_btn.h
  libs/lvgl/src/lv_objx/lv_btn.c
  libs/lvgl/src/lv_objx/lv_cont.h
  libs/lvgl/src/lv_objx/lv_cont.c
  libs/lvgl/src/lv_objx/lv_label.h
  libs/lvgl/src/lv_objx/lv_label.c
  libs/lvgl/src/lv_themes/lv_theme.c
  libs/lvgl/src/lv_themes/lv_theme.h
  libs/lvgl/src/lv_themes/lv_theme_night.h
  libs/lvgl/src/lv_themes/lv_theme_night.c
  libs/lvgl/src/lv_objx/lv_list.c
  libs/lvgl/src/lv_objx/lv_list.h
  libs/lvgl/src/lv_objx/lv_tileview.c
  libs/lvgl/src/lv_objx/lv_tileview.h
  libs/lvgl/src/lv_objx/lv_tabview.c
  libs/lvgl/src/lv_objx/lv_tabview.h
  libs/lvgl/src/lv_objx/lv_btnm.c
  libs/lvgl/src/lv_objx/lv_btnm.h
  libs/lvgl/src/lv_objx/lv_page.c
  libs/lvgl/src/lv_objx/lv_page.h
  libs/lvgl/src/lv_objx/lv_img.c
  libs/lvgl/src/lv_objx/lv_img.h
  libs/lvgl/src/lv_objx/lv_lmeter.c
  libs/lvgl/src/lv_objx/lv_lmeter.h
  libs/lvgl/src/lv_objx/lv_arc.c
  libs/lvgl/src/lv_objx/lv_arc.h
  libs/lvgl/src/lv_objx/lv_gauge.c
  libs/lvgl/src/lv_objx/lv_gauge.h
  libs/lvgl/src/lv_objx/lv_mbox.c
  libs/lvgl/src/lv_objx/lv_mbox.h
  libs/lvgl/src/lv_objx/lv_bar.c
  libs/lvgl/src/lv_objx/lv_bar.h
  libs/lvgl/src/lv_objx/lv_slider.h
  libs/lvgl/src/lv_objx/lv_slider.c
  )

list(APPEND IMAGE_FILES
  DisplayApp/Icons/battery/os_battery_error.c
  DisplayApp/Icons/battery/os_battery_100.c
  DisplayApp/Icons/battery/os_battery_090.c
  DisplayApp/Icons/battery/os_battery_080.c
  DisplayApp/Icons/battery/os_battery_070.c
  DisplayApp/Icons/battery/os_battery_060.c
  DisplayApp/Icons/battery/os_battery_050.c
  DisplayApp/Icons/battery/os_battery_040.c
  DisplayApp/Icons/battery/os_battery_030.c
  DisplayApp/Icons/battery/os_battery_020.c
  DisplayApp/Icons/battery/os_battery_010.c
  DisplayApp/Icons/battery/os_battery_005.c

  DisplayApp/Icons/battery/os_batterycharging_100.c
  DisplayApp/Icons/battery/os_batterycharging_090.c
  DisplayApp/Icons/battery/os_batterycharging_080.c
  DisplayApp/Icons/battery/os_batterycharging_070.c
  DisplayApp/Icons/battery/os_batterycharging_060.c
  DisplayApp/Icons/battery/os_batterycharging_050.c
  DisplayApp/Icons/battery/os_batterycharging_040.c
  DisplayApp/Icons/battery/os_batterycharging_030.c
  DisplayApp/Icons/battery/os_batterycharging_020.c
  DisplayApp/Icons/battery/os_batterycharging_010.c
  DisplayApp/Icons/battery/os_batterycharging_005.c

  DisplayApp/Icons/bluetooth/os_bt_connected.c
  DisplayApp/Icons/bluetooth/os_bt_disconnected.c
  )

list(APPEND SOURCE_FILES
  Logging/NrfLogger.cpp
  BlinkApp/BlinkApp.cpp
  DisplayApp/DisplayApp.cpp
  DisplayApp/Screens/Screen.cpp
  DisplayApp/Screens/Clock.cpp
  DisplayApp/Screens/Message.cpp
  DisplayApp/Screens/Tile.cpp
  DisplayApp/Screens/Meter.cpp
  DisplayApp/Screens/Gauge.cpp
  DisplayApp/Screens/Modal.cpp
  DisplayApp/Screens/BatteryIcon.cpp
  DisplayApp/Screens/BleIcon.cpp
  DisplayApp/Screens/Brightness.cpp
  DisplayApp/Screens/ScreenList.cpp
  DisplayApp/Screens/Label.cpp
  DisplayApp/Screens/FirmwareUpdate.cpp
  DisplayApp/Screens/Music.cpp
  main.cpp
  drivers/St7789.cpp
  drivers/SpiNorFlash.cpp
  drivers/SpiMaster.cpp
  drivers/Spi.cpp
  drivers/Watchdog.cpp
  drivers/DebugPins.cpp
  drivers/InternalFlash.cpp
  Components/Battery/BatteryController.cpp
  Components/Ble/BleController.cpp
  Components/Ble/NotificationManager.cpp
  Components/DateTime/DateTimeController.cpp
  Components/Brightness/BrightnessController.cpp
  Components/Ble/NimbleController.cpp
  Components/Ble/DeviceInformationService.cpp
  Components/Ble/CurrentTimeClient.cpp
  Components/Ble/AlertNotificationClient.cpp
  Components/Ble/DfuService.cpp
  Components/Ble/CurrentTimeService.cpp
  Components/Ble/AlertNotificationService.cpp
  Components/Ble/MusicService.cpp
  drivers/Cst816s.cpp
  FreeRTOS/port.c
  FreeRTOS/port_cmsis_systick.c
  FreeRTOS/port_cmsis.c
  ${TINYCRYPT_SRC}
  ${NIMBLE_SRC}
  ${LVGL_SRC}
  #${IMAGE_FILES}
  ${SDK_SOURCE_FILES}

  DisplayApp/LittleVgl.cpp
  DisplayApp/Fonts/jetbrains_mono_extrabold_compressed.c
  DisplayApp/Fonts/jetbrains_mono_bold_20.c

  SystemTask/SystemTask.cpp
    drivers/TwiMaster.cpp
)

list(APPEND GRAPHICS_SOURCE_FILES
    ${SDK_SOURCE_FILES}

    # FreeRTOS
    FreeRTOS/port.c
    FreeRTOS/port_cmsis_systick.c
    FreeRTOS/port_cmsis.c

    drivers/SpiNorFlash.cpp
    drivers/SpiMaster.cpp
    drivers/Spi.cpp
    Logging/NrfLogger.cpp

    Components/Gfx/Gfx.cpp
    drivers/St7789.cpp
    Components/Brightness/BrightnessController.cpp

    graphics.cpp
)

set(INCLUDE_FILES
  Logging/Logger.h
  Logging/NrfLogger.h
  BlinkApp/BlinkApp.h
  DisplayApp/DisplayApp.h
  DisplayApp/TouchEvents.h
  DisplayApp/Screens/Screen.h
  DisplayApp/Screens/Clock.h
  DisplayApp/Screens/Message.h
  DisplayApp/Screens/Tile.h
  DisplayApp/Screens/Meter.h
  DisplayApp/Screens/Gauge.h
  DisplayApp/Screens/Modal.h
  DisplayApp/Screens/BatteryIcon.h
  DisplayApp/Screens/BleIcon.cpp
  DisplayApp/Screens/Brightness.h
  DisplayApp/Screens/ScreenList.h
  DisplayApp/Screens/Label.h
  DisplayApp/Screens/FirmwareUpdate.h
  drivers/St7789.h
  drivers/SpiNorFlash.h
  drivers/SpiMaster.h
  drivers/Spi.h
  drivers/Watchdog.h
  drivers/DebugPins.h
  drivers/InternalFlash.h
  Components/Battery/BatteryController.h
  Components/Ble/BleController.h
  Components/Ble/NotificationManager.h
  Components/DateTime/DateTimeController.h
  Components/Brightness/BrightnessController.h
  Components/Ble/NimbleController.h
  Components/Ble/DeviceInformationService.h
  Components/Ble/CurrentTimeClient.h
  Components/Ble/AlertNotificationClient.h
  Components/Ble/DfuService.h
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
  DisplayApp/LittleVgl.h
  SystemTask/SystemTask.h
  SystemTask/SystemMonitor.h
  DisplayApp/Screens/Symbols.h
    drivers/TwiMaster.h
)

include_directories(
  .
  ../
  libs/
  FreeRTOS/
  libs/date/includes
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
        ${NRF5_SDK_PATH}/external/freertos/source/include
        "${NRF5_SDK_PATH}/components/toolchain/cmsis/include"
        "${NRF5_SDK_PATH}/components/libraries/atomic"
        "${NRF5_SDK_PATH}/components/libraries/atomic_fifo"
        "${NRF5_SDK_PATH}/components/libraries/atomic_flags"
        "${NRF5_SDK_PATH}/components/libraries/balloc"
        "${NRF5_SDK_PATH}/components/libraries/bootloader/ble_dfu"
        "${NRF5_SDK_PATH}/components/libraries/cli"
        "${NRF5_SDK_PATH}/components/libraries/crc16"
        "${NRF5_SDK_PATH}/components/libraries/crc32"
        "${NRF5_SDK_PATH}/components/libraries/crypto"
        "${NRF5_SDK_PATH}/components/libraries/csense"
        "${NRF5_SDK_PATH}/components/libraries/csense_drv"
        "${NRF5_SDK_PATH}/components/libraries/delay"
        "${NRF5_SDK_PATH}/components/libraries/ecc"
        "${NRF5_SDK_PATH}/components/libraries/experimental_section_vars"
        "${NRF5_SDK_PATH}/components/libraries/experimental_task_manager"
        "${NRF5_SDK_PATH}/components/libraries/fds"
        "${NRF5_SDK_PATH}/components/libraries/fstorage"
        "${NRF5_SDK_PATH}/components/libraries/gfx"
        "${NRF5_SDK_PATH}/components/libraries/gpiote"
        "${NRF5_SDK_PATH}/components/libraries/hardfault"
        "${NRF5_SDK_PATH}/components/libraries/hci"
        "${NRF5_SDK_PATH}/components/libraries/led_softblink"
        "${NRF5_SDK_PATH}/components/libraries/log"
        "${NRF5_SDK_PATH}/components/libraries/log/src"
        "${NRF5_SDK_PATH}/components/libraries/low_power_pwm"
        "${NRF5_SDK_PATH}/components/libraries/mem_manager"
        "${NRF5_SDK_PATH}/components/libraries/memobj"
        "${NRF5_SDK_PATH}/components/libraries/mpu"
        "${NRF5_SDK_PATH}/components/libraries/mutex"
        "${NRF5_SDK_PATH}/components/libraries/pwm"
        "${NRF5_SDK_PATH}/components/libraries/pwr_mgmt"
        "${NRF5_SDK_PATH}/components/libraries/queue"
        "${NRF5_SDK_PATH}/components/libraries/ringbuf"
        "${NRF5_SDK_PATH}/components/libraries/scheduler"
        "${NRF5_SDK_PATH}/components/libraries/sdcard"
        "${NRF5_SDK_PATH}/components/libraries/slip"
        "${NRF5_SDK_PATH}/components/libraries/sortlist"
        "${NRF5_SDK_PATH}/components/libraries/spi_mngr"
        "${NRF5_SDK_PATH}/components/libraries/stack_guard"
        "${NRF5_SDK_PATH}/components/libraries/strerror"
        "${NRF5_SDK_PATH}/components/libraries/svc"
        "${NRF5_SDK_PATH}/components/libraries/timer"
        "${NRF5_SDK_PATH}/components/libraries/usbd"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/audio"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc/acm"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/generic"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/kbd"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/mouse"
        "${NRF5_SDK_PATH}/components/libraries/usbd/class/msc"
        "${NRF5_SDK_PATH}/components/libraries/util"
        "${NRF5_SDK_PATH}/external/segger_rtt/"
        "${NRF5_SDK_PATH}/external/fprintf/"
        "${NRF5_SDK_PATH}/external/thedotfactory_fonts"
        "${NRF5_SDK_PATH}/components/libraries/gpiote"

)

link_directories(
	../
)


set(COMMON_FLAGS -MP -MD -mthumb -mabi=aapcs -Wall -g3 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin --short-enums -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wreturn-type -Werror=return-type)
add_definitions(-DCONFIG_GPIO_AS_PINRESET)
add_definitions(-DDEBUG)
add_definitions(-DNIMBLE_CFG_CONTROLLER)
add_definitions(-DOS_CPUTIME_FREQ)
add_definitions(-DNRF52 -DNRF52832 -DNRF52832_XXAA -DNRF52_PAN_74 -DNRF52_PAN_64 -DNRF52_PAN_12 -DNRF52_PAN_58 -DNRF52_PAN_54 -DNRF52_PAN_31 -DNRF52_PAN_51 -DNRF52_PAN_36 -DNRF52_PAN_15 -DNRF52_PAN_20 -DNRF52_PAN_55 -DBOARD_PCA10040)
add_definitions(-DFREERTOS)
add_definitions(-DDEBUG_NRF_USER)
add_definitions(-D__STACK_SIZE=8192)
add_definitions(-D__HEAP_SIZE=8192)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Build autonomous binary (without support for bootloader)
set(EXECUTABLE_NAME "pinetime-app")
set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/gcc_nrf52.ld")
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})
target_compile_options(${EXECUTABLE_NAME} PUBLIC
        $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:DEBUG>>: ${COMMON_FLAGS} -O0 -g3>
        $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELEASE>>: ${COMMON_FLAGS} -O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:DEBUG>>: ${COMMON_FLAGS} -O0 -g3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELEASE>>: ${COMMON_FLAGS} -O3>
        $<$<COMPILE_LANGUAGE:ASM>: -MP -MD -std=c99 -x assembler-with-cpp>
)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -std=gnu++98 -std=c99 -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_NAME}.map"
        CXX_STANDARD 11
        C_STANDARD 99
        )

add_custom_command(TARGET ${EXECUTABLE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_NAME}.out "${EXECUTABLE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_NAME}.out "${EXECUTABLE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_NAME}")


# Build binary intended to be used by bootloader
set(EXECUTABLE_MCUBOOT_NAME "pinetime-mcuboot-app")
set(EXECUTABLE_MCUBOOT_WITH_BOOTLOADER_NAME "pinetime-mcuboot-app-wth-bootloader")
set(NRF5_LINKER_SCRIPT_MCUBOOT "${CMAKE_SOURCE_DIR}/gcc_nrf52-mcuboot.ld")
add_executable(${EXECUTABLE_MCUBOOT_NAME} ${SOURCE_FILES})
target_compile_options(${EXECUTABLE_MCUBOOT_NAME} PUBLIC
        $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:DEBUG>>: ${COMMON_FLAGS} -O0 -g3>
        $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELEASE>>: ${COMMON_FLAGS} -O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:DEBUG>>: ${COMMON_FLAGS} -O0 -g3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELEASE>>: ${COMMON_FLAGS} -O3>
        $<$<COMPILE_LANGUAGE:ASM>: -MP -MD -std=c99 -x assembler-with-cpp>
        )

set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -std=gnu++98 -std=c99 -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_MCUBOOT_NAME}.map"
        CXX_STANDARD 11
        C_STANDARD 99
        )

add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_MCUBOOT_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_MCUBOOT_NAME}.out "${EXECUTABLE_MCUBOOT_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_MCUBOOT_NAME}.out "${EXECUTABLE_MCUBOOT_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_MCUBOOT_NAME}"
)

# Build binary that writes the graphic assets for the bootloader
set(EXECUTABLE_GRAPHICS_NAME "pinetime-graphics")
add_executable(${EXECUTABLE_GRAPHICS_NAME} ${GRAPHICS_SOURCE_FILES})
target_compile_options(${EXECUTABLE_GRAPHICS_NAME} PUBLIC
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:DEBUG>>: ${COMMON_FLAGS} -O0 -g3>
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:RELEASE>>: ${COMMON_FLAGS} -O3>
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:DEBUG>>: ${COMMON_FLAGS} -O0 -g3>
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RELEASE>>: ${COMMON_FLAGS} -O3>
    $<$<COMPILE_LANGUAGE:ASM>: -MP -MD -std=c99 -x assembler-with-cpp>
    )

set_target_properties(${EXECUTABLE_GRAPHICS_NAME} PROPERTIES
    SUFFIX ".out"
    LINK_FLAGS "-mthumb -mabi=aapcs -std=gnu++98 -std=c99 -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_GRAPHICS_NAME}.map"
    CXX_STANDARD 11
    C_STANDARD 99
    )

add_custom_command(TARGET ${EXECUTABLE_GRAPHICS_NAME}
    POST_BUILD
    COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_GRAPHICS_NAME}.out
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_GRAPHICS_NAME}.out "${EXECUTABLE_GRAPHICS_NAME}.bin"
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_GRAPHICS_NAME}.out "${EXECUTABLE_GRAPHICS_NAME}.hex"
    COMMENT "post build steps for ${EXECUTABLE_GRAPHICS_NAME}"
    )

# FLASH
if(USE_JLINK)
    add_custom_target(FLASH_ERASE
            COMMAND ${NRFJPROG} --eraseall -f ${NRF_TARGET}
            COMMENT "erasing flashing"
            )
    add_custom_target("FLASH_${EXECUTABLE_NAME}"
            DEPENDS ${EXECUTABLE_NAME}
            COMMAND ${NRFJPROG} --program ${EXECUTABLE_NAME}.hex -f ${NRF_TARGET} --sectorerase
            COMMAND sleep 0.5s
            COMMAND ${NRFJPROG} --reset -f ${NRF_TARGET}
            COMMENT "flashing ${EXECUTABLE_NAME}.hex"
            )

elseif(USE_GDB_CLIENT)
    add_custom_target(FLASH_ERASE
            COMMAND ${GDB_CLIENT_BIN_PATH} -nx --batch -ex 'target extended-remote ${GDB_CLIENT_TARGET_REMOTE}' -ex 'monitor swdp_scan' -ex 'attach 1' -ex 'mon erase_mass'
            COMMENT "erasing flashing"
            )
    add_custom_target("FLASH_${EXECUTABLE_NAME}"
            DEPENDS ${EXECUTABLE_NAME}
            COMMAND ${GDB_CLIENT_BIN_PATH} -nx --batch -ex 'target extended-remote ${GDB_CLIENT_TARGET_REMOTE}' -ex 'monitor swdp_scan' -ex 'attach 1' -ex 'load' -ex 'kill'  ${EXECUTABLE_NAME}.hex
            COMMENT "flashing ${EXECUTABLE_NAME}.hex"
            )
elseif(USE_OPENOCD)
    add_custom_target(FLASH_ERASE
            COMMAND ${OPENOCD_BIN_PATH}  -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c init -c halt -c 'nrf5 mass_erase' -c reset -c shutdown
            COMMENT "erasing flashing"
            )
    add_custom_target("FLASH_${EXECUTABLE_NAME}"
            DEPENDS ${EXECUTABLE_NAME}
            COMMAND ${OPENOCD_BIN_PATH} -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c "program \"${EXECUTABLE_NAME}.hex\""  -c reset -c shutdown
            COMMENT "flashing ${EXECUTABLE_NAME}.hex"
            )

endif()
