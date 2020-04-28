cmake_minimum_required(VERSION 3.10)

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
add_definitions(-DNIMBLE_CFG_CONTROLLER)
add_definitions(-DOS_CPUTIME_FREQ)

include_directories(.)
include_directories(libs/)

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
#  libs/lvgl/src/lv_font/lv_font_roboto_16.c
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
  main.cpp
  drivers/St7789.cpp
  drivers/SpiMaster.cpp
  drivers/Watchdog.cpp
  drivers/DebugPins.cpp
  Components/Battery/BatteryController.cpp
  Components/Ble/BleController.cpp
  Components/Ble/NotificationManager.cpp
  Components/DateTime/DateTimeController.cpp
  Components/Brightness/BrightnessController.cpp
  Components/Ble/NimbleController.cpp
  Components/Ble/DeviceInformationService.cpp
  Components/Ble/CurrentTimeClient.cpp
  Components/Ble/AlertNotificationClient.cpp
  Components/Ble/PinetimeService.cpp
  drivers/Cst816s.cpp
  FreeRTOS/port.c
  FreeRTOS/port_cmsis_systick.c
  FreeRTOS/port_cmsis.c
  ${TINYCRYPT_SRC}
  ${NIMBLE_SRC}
  ${LVGL_SRC}
  ${IMAGE_FILES}

  DisplayApp/LittleVgl.cpp
  DisplayApp/Fonts/jetbrains_mono_extrabold_compressed.c
  DisplayApp/Fonts/jetbrains_mono_bold_20.c

  SystemTask/SystemTask.cpp
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
  drivers/St7789.h
  drivers/SpiMaster.h
  drivers/Watchdog.h
  drivers/DebugPins.h
  BLE/BleManager.h
  Components/Battery/BatteryController.h
  Components/Ble/BleController.h
  Components/Ble/NotificationManager.h
  Components/DateTime/DateTimeController.h
  Components/Brightness/BrightnessController.h
  Components/Ble/NimbleController.h
  Components/Ble/DeviceInformationService.h
  Components/Ble/CurrentTimeClient.h
  Components/Ble/AlertNotificationClient.h
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
)

include_directories(
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
)

link_directories(
  ../
)

nRF5x_addExecutable(pinetime-app "${SOURCE_FILES}" ${INCLUDE_FILES})
