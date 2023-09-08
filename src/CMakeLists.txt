cmake_minimum_required(VERSION 3.10)

project(pinetime-app C CXX ASM)


# define some variables just for this example to determine file locations
set(NRF_PROJECT_NAME pinetime-app)
set(NRF_BOARD pca10040)

# check if all the necessary tools paths have been provided.
if (NOT NRF5_SDK_PATH)
  message(FATAL_ERROR "The path to the nRF5 SDK (NRF5_SDK_PATH) must be set.")
endif ()
if (DEFINED ARM_NONE_EABI_TOOLCHAIN_PATH)
  set(ARM_NONE_EABI_TOOLCHAIN_BIN_PATH ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin)
endif ()

if (NOT NRF_TARGET MATCHES "nrf52")
  message(FATAL_ERROR "Only rRF52 boards are supported right now")
endif ()

# Setup toolchain
include(${CMAKE_SOURCE_DIR}/cmake-nRF5x/arm-gcc-toolchain.cmake)

if (NOT DEFINED ARM_GCC_TOOLCHAIN)
  message(FATAL_ERROR "The toolchain must be set up before calling this macro")
endif ()
set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")


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
        libs/mynewt-nimble/nimble/host/src/ble_att_svr.c
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

set(LITTLEFS_SRC
        libs/littlefs/lfs_util.h
        libs/littlefs/lfs.h
        libs/littlefs/lfs_util.c
        libs/littlefs/lfs.c
        )

set(LVGL_SRC
        libs/lv_conf.h
        libs/lvgl/lvgl.h

        libs/lvgl/src/lvgl.h
        libs/lvgl/src/lv_api_map.h
        libs/lvgl/src/lv_conf_internal.h
        libs/lvgl/src/lv_core/lv_disp.h
        libs/lvgl/src/lv_core/lv_group.h
        libs/lvgl/src/lv_core/lv_indev.h
        libs/lvgl/src/lv_core/lv_obj.h
        libs/lvgl/src/lv_core/lv_obj_style_dec.h
        libs/lvgl/src/lv_core/lv_refr.h
        libs/lvgl/src/lv_core/lv_style.h
        libs/lvgl/src/lv_draw/lv_draw.h
        libs/lvgl/src/lv_draw/lv_draw_arc.h
        libs/lvgl/src/lv_draw/lv_draw_blend.h
        libs/lvgl/src/lv_draw/lv_draw_img.h
        libs/lvgl/src/lv_draw/lv_draw_label.h
        libs/lvgl/src/lv_draw/lv_draw_line.h
        libs/lvgl/src/lv_draw/lv_draw_mask.h
        libs/lvgl/src/lv_draw/lv_draw_rect.h
        libs/lvgl/src/lv_draw/lv_draw_triangle.h
        libs/lvgl/src/lv_draw/lv_img_buf.h
        libs/lvgl/src/lv_draw/lv_img_cache.h
        libs/lvgl/src/lv_draw/lv_img_decoder.h
        libs/lvgl/src/lv_font/lv_font.h
        libs/lvgl/src/lv_font/lv_font_fmt_txt.h
        libs/lvgl/src/lv_font/lv_font_loader.h
        libs/lvgl/src/lv_font/lv_symbol_def.h
        libs/lvgl/src/lv_hal/lv_hal.h
        libs/lvgl/src/lv_hal/lv_hal_disp.h
        libs/lvgl/src/lv_hal/lv_hal_indev.h
        libs/lvgl/src/lv_hal/lv_hal_tick.h
        libs/lvgl/src/lv_misc/lv_anim.h
        libs/lvgl/src/lv_misc/lv_area.h
        libs/lvgl/src/lv_misc/lv_async.h
        libs/lvgl/src/lv_misc/lv_bidi.h
        libs/lvgl/src/lv_misc/lv_color.h
        libs/lvgl/src/lv_misc/lv_debug.h
        libs/lvgl/src/lv_misc/lv_fs.h
        libs/lvgl/src/lv_misc/lv_gc.h
        libs/lvgl/src/lv_misc/lv_ll.h
        libs/lvgl/src/lv_misc/lv_log.h
        libs/lvgl/src/lv_misc/lv_math.h
        libs/lvgl/src/lv_misc/lv_mem.h
        libs/lvgl/src/lv_misc/lv_printf.h
        libs/lvgl/src/lv_misc/lv_task.h
        libs/lvgl/src/lv_misc/lv_templ.h
        libs/lvgl/src/lv_misc/lv_txt.h
        libs/lvgl/src/lv_misc/lv_txt_ap.h
        libs/lvgl/src/lv_misc/lv_types.h
        libs/lvgl/src/lv_misc/lv_utils.h
        libs/lvgl/src/lv_themes/lv_theme.h
        libs/lvgl/src/lv_themes/lv_theme_empty.h
        libs/lvgl/src/lv_themes/lv_theme_material.h
        #libs/lvgl/src/lv_themes/lv_theme_mono.h
        #libs/lvgl/src/lv_themes/lv_theme_template.h
        libs/lvgl/src/lv_widgets/lv_arc.h
        libs/lvgl/src/lv_widgets/lv_bar.h
        libs/lvgl/src/lv_widgets/lv_btn.h
        libs/lvgl/src/lv_widgets/lv_btnmatrix.h
        libs/lvgl/src/lv_widgets/lv_calendar.h
        libs/lvgl/src/lv_widgets/lv_canvas.h
        libs/lvgl/src/lv_widgets/lv_chart.h
        libs/lvgl/src/lv_widgets/lv_checkbox.h
        libs/lvgl/src/lv_widgets/lv_cont.h
        libs/lvgl/src/lv_widgets/lv_cpicker.h
        libs/lvgl/src/lv_widgets/lv_dropdown.h
        libs/lvgl/src/lv_widgets/lv_gauge.h
        libs/lvgl/src/lv_widgets/lv_img.h
        libs/lvgl/src/lv_widgets/lv_imgbtn.h
        libs/lvgl/src/lv_widgets/lv_keyboard.h
        libs/lvgl/src/lv_widgets/lv_label.h
        libs/lvgl/src/lv_widgets/lv_led.h
        libs/lvgl/src/lv_widgets/lv_line.h
        libs/lvgl/src/lv_widgets/lv_linemeter.h
        libs/lvgl/src/lv_widgets/lv_list.h
        libs/lvgl/src/lv_widgets/lv_msgbox.h
        libs/lvgl/src/lv_widgets/lv_objmask.h
        libs/lvgl/src/lv_widgets/lv_objx_templ.h
        libs/lvgl/src/lv_widgets/lv_page.h
        libs/lvgl/src/lv_widgets/lv_roller.h
        libs/lvgl/src/lv_widgets/lv_slider.h
        libs/lvgl/src/lv_widgets/lv_spinbox.h
        libs/lvgl/src/lv_widgets/lv_spinner.h
        libs/lvgl/src/lv_widgets/lv_switch.h
        libs/lvgl/src/lv_widgets/lv_table.h
        libs/lvgl/src/lv_widgets/lv_tabview.h
        libs/lvgl/src/lv_widgets/lv_textarea.h
        libs/lvgl/src/lv_widgets/lv_tileview.h
        libs/lvgl/src/lv_widgets/lv_win.h
        libs/lvgl/src/lv_core/lv_disp.c
        libs/lvgl/src/lv_core/lv_group.c
        libs/lvgl/src/lv_core/lv_indev.c
        libs/lvgl/src/lv_core/lv_obj.c
        libs/lvgl/src/lv_core/lv_refr.c
        libs/lvgl/src/lv_core/lv_style.c
        libs/lvgl/src/lv_draw/lv_draw_arc.c
        libs/lvgl/src/lv_draw/lv_draw_blend.c
        libs/lvgl/src/lv_draw/lv_draw_img.c
        libs/lvgl/src/lv_draw/lv_draw_label.c
        libs/lvgl/src/lv_draw/lv_draw_line.c
        libs/lvgl/src/lv_draw/lv_draw_mask.c
        libs/lvgl/src/lv_draw/lv_draw_rect.c
        libs/lvgl/src/lv_draw/lv_draw_triangle.c
        libs/lvgl/src/lv_draw/lv_img_buf.c
        libs/lvgl/src/lv_draw/lv_img_cache.c
        libs/lvgl/src/lv_draw/lv_img_decoder.c
        libs/lvgl/src/lv_font/lv_font.c
        #libs/lvgl/src/lv_font/lv_font_dejavu_16_persian_hebrew.c
        libs/lvgl/src/lv_font/lv_font_fmt_txt.c
        libs/lvgl/src/lv_font/lv_font_loader.c
        # LVGL Fonts
        libs/lvgl/src/lv_font/lv_font_montserrat_14.c
        libs/lvgl/src/lv_font/lv_font_montserrat_18.c
        libs/lvgl/src/lv_font/lv_font_montserrat_22.c
        libs/lvgl/src/lv_font/lv_font_montserrat_28.c
        #
        libs/lvgl/src/lv_hal/lv_hal_disp.c
        libs/lvgl/src/lv_hal/lv_hal_indev.c
        libs/lvgl/src/lv_hal/lv_hal_tick.c
        libs/lvgl/src/lv_misc/lv_anim.c
        libs/lvgl/src/lv_misc/lv_area.c
        libs/lvgl/src/lv_misc/lv_async.c
        libs/lvgl/src/lv_misc/lv_bidi.c
        libs/lvgl/src/lv_misc/lv_color.c
        libs/lvgl/src/lv_misc/lv_debug.c
        libs/lvgl/src/lv_misc/lv_fs.c
        libs/lvgl/src/lv_misc/lv_gc.c
        libs/lvgl/src/lv_misc/lv_ll.c
        libs/lvgl/src/lv_misc/lv_log.c
        libs/lvgl/src/lv_misc/lv_math.c
        libs/lvgl/src/lv_misc/lv_mem.c
        libs/lvgl/src/lv_misc/lv_printf.c
        libs/lvgl/src/lv_misc/lv_task.c
        libs/lvgl/src/lv_misc/lv_templ.c
        libs/lvgl/src/lv_misc/lv_txt.c
        libs/lvgl/src/lv_misc/lv_txt_ap.c
        libs/lvgl/src/lv_misc/lv_utils.c
        libs/lvgl/src/lv_themes/lv_theme.c
        libs/lvgl/src/lv_themes/lv_theme_empty.c
        libs/lvgl/src/lv_themes/lv_theme_material.c
        #libs/lvgl/src/lv_themes/lv_theme_mono.c
        #libs/lvgl/src/lv_themes/lv_theme_template.c
        libs/lvgl/src/lv_widgets/lv_arc.c
        libs/lvgl/src/lv_widgets/lv_bar.c
        libs/lvgl/src/lv_widgets/lv_btn.c
        libs/lvgl/src/lv_widgets/lv_btnmatrix.c
        libs/lvgl/src/lv_widgets/lv_calendar.c
        libs/lvgl/src/lv_widgets/lv_canvas.c
        libs/lvgl/src/lv_widgets/lv_chart.c
        libs/lvgl/src/lv_widgets/lv_checkbox.c
        libs/lvgl/src/lv_widgets/lv_cont.c
        libs/lvgl/src/lv_widgets/lv_cpicker.c
        libs/lvgl/src/lv_widgets/lv_dropdown.c
        libs/lvgl/src/lv_widgets/lv_gauge.c
        libs/lvgl/src/lv_widgets/lv_img.c
        libs/lvgl/src/lv_widgets/lv_imgbtn.c
        libs/lvgl/src/lv_widgets/lv_keyboard.c
        libs/lvgl/src/lv_widgets/lv_label.c
        libs/lvgl/src/lv_widgets/lv_led.c
        libs/lvgl/src/lv_widgets/lv_line.c
        libs/lvgl/src/lv_widgets/lv_linemeter.c
        libs/lvgl/src/lv_widgets/lv_list.c
        libs/lvgl/src/lv_widgets/lv_msgbox.c
        libs/lvgl/src/lv_widgets/lv_objmask.c
        libs/lvgl/src/lv_widgets/lv_objx_templ.c
        libs/lvgl/src/lv_widgets/lv_page.c
        libs/lvgl/src/lv_widgets/lv_roller.c
        libs/lvgl/src/lv_widgets/lv_slider.c
        libs/lvgl/src/lv_widgets/lv_spinbox.c
        libs/lvgl/src/lv_widgets/lv_spinner.c
        libs/lvgl/src/lv_widgets/lv_switch.c
        libs/lvgl/src/lv_widgets/lv_table.c
        libs/lvgl/src/lv_widgets/lv_tabview.c
        libs/lvgl/src/lv_widgets/lv_textarea.c
        libs/lvgl/src/lv_widgets/lv_tileview.c
        libs/lvgl/src/lv_widgets/lv_win.c
        )

set(QCBOR_SRC
        libs/QCBOR/src/ieee754.c
        libs/QCBOR/src/qcbor_decode.c
        libs/QCBOR/src/qcbor_encode.c
        libs/QCBOR/src/qcbor_err_to_str.c
        libs/QCBOR/src/UsefulBuf.c
        )

list(APPEND IMAGE_FILES
        displayapp/icons/battery/batteryicon.c
        )
list(APPEND SOURCE_FILES
        stdlib.c
        FreeRTOS/heap_4_infinitime.c
        BootloaderVersion.cpp
        logging/NrfLogger.cpp
        displayapp/DisplayApp.cpp
        displayapp/screens/Screen.cpp
        displayapp/screens/Clock.cpp
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
        displayapp/screens/Weather.cpp
        displayapp/screens/Navigation.cpp
        displayapp/screens/Metronome.cpp
        displayapp/screens/Motion.cpp
        displayapp/screens/FirmwareValidation.cpp
        displayapp/screens/ApplicationList.cpp
        displayapp/screens/Notifications.cpp
        displayapp/screens/Twos.cpp
        displayapp/screens/HeartRate.cpp
        displayapp/screens/Motion.cpp
        displayapp/screens/FlashLight.cpp
        displayapp/screens/List.cpp
        displayapp/screens/CheckboxList.cpp
        displayapp/screens/BatteryInfo.cpp
        displayapp/screens/Steps.cpp
        displayapp/screens/Timer.cpp
        displayapp/screens/PassKey.cpp
        displayapp/screens/Error.cpp
        displayapp/screens/Alarm.cpp
        displayapp/screens/Styles.cpp
        displayapp/Colors.cpp
        displayapp/widgets/Counter.cpp
        displayapp/widgets/PageIndicator.cpp
        displayapp/widgets/DotIndicator.cpp
        displayapp/widgets/StatusIcons.cpp
        displayapp/widgets/PopupMessage.cpp

        ## Settings
        displayapp/screens/settings/QuickSettings.cpp
        displayapp/screens/settings/Settings.cpp
        displayapp/screens/settings/SettingWatchFace.cpp
        displayapp/screens/settings/SettingTimeFormat.cpp
        displayapp/screens/settings/SettingWakeUp.cpp
        displayapp/screens/settings/SettingDisplay.cpp
        displayapp/screens/settings/SettingSteps.cpp
        displayapp/screens/settings/SettingSetDateTime.cpp
        displayapp/screens/settings/SettingSetDate.cpp
        displayapp/screens/settings/SettingSetTime.cpp
        displayapp/screens/settings/SettingChimes.cpp
        displayapp/screens/settings/SettingShakeThreshold.cpp
        displayapp/screens/settings/SettingBluetooth.cpp

        ## Watch faces
        displayapp/screens/WatchFaceAnalog.cpp
        displayapp/screens/WatchFaceDigital.cpp
        displayapp/screens/WatchFaceInfineat.cpp
        displayapp/screens/WatchFaceTerminal.cpp
        displayapp/screens/WatchFacePineTimeStyle.cpp
        displayapp/screens/WatchFaceCasioStyleG7710.cpp

        ##

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
        components/ble/weather/WeatherService.cpp
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
        drivers/TwiMaster.cpp

        heartratetask/HeartRateTask.cpp
        components/heartrate/HeartRateController.cpp
        components/heartrate/Ppg.cpp

        buttonhandler/ButtonHandler.cpp
        touchhandler/TouchHandler.cpp

        utility/Math.cpp
        )

list(APPEND RECOVERY_SOURCE_FILES
        stdlib.c
        FreeRTOS/heap_4_infinitime.c

        BootloaderVersion.cpp
        logging/NrfLogger.cpp
        displayapp/DisplayAppRecovery.cpp

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
        components/ble/weather/WeatherService.cpp
        components/ble/BatteryInformationService.cpp
        components/ble/FSService.cpp
        components/ble/ImmediateAlertService.cpp
        components/ble/ServiceDiscovery.cpp
        components/ble/NavigationService.cpp
        components/ble/HeartRateService.cpp
        components/ble/MotionService.cpp
        components/firmwarevalidator/FirmwareValidator.cpp
        components/settings/Settings.cpp
        components/timer/Timer.cpp
        components/alarm/AlarmController.cpp
        drivers/Cst816s.cpp
        FreeRTOS/port.c
        FreeRTOS/port_cmsis_systick.c
        FreeRTOS/port_cmsis.c

        systemtask/SystemTask.cpp
        systemtask/SystemMonitor.cpp
        drivers/TwiMaster.cpp
        components/gfx/Gfx.cpp
        components/rle/RleDecoder.cpp
        components/heartrate/HeartRateController.cpp
        heartratetask/HeartRateTask.cpp
        components/heartrate/Ppg.cpp

        components/motor/MotorController.cpp
        components/fs/FS.cpp
        buttonhandler/ButtonHandler.cpp
        touchhandler/TouchHandler.cpp

        utility/Math.cpp
        )

list(APPEND RECOVERYLOADER_SOURCE_FILES
        stdlib.c
        FreeRTOS/heap_4_infinitime.c

        # FreeRTOS
        FreeRTOS/port.c
        FreeRTOS/port_cmsis_systick.c
        FreeRTOS/port_cmsis.c

        drivers/SpiNorFlash.cpp
        drivers/SpiMaster.cpp
        drivers/Spi.cpp
        logging/NrfLogger.cpp

        components/rle/RleDecoder.cpp

        components/gfx/Gfx.cpp
        drivers/St7789.cpp
        components/brightness/BrightnessController.cpp

        recoveryLoader.cpp
        )


set(INCLUDE_FILES

        ${CMAKE_CURRENT_BINARY_DIR}/src/Version.h
        BootloaderVersion.h
        logging/Logger.h
        logging/NrfLogger.h
        displayapp/DisplayApp.h
        displayapp/Messages.h
        displayapp/TouchEvents.h
        displayapp/screens/Screen.h
        displayapp/screens/Clock.h
        displayapp/screens/Tile.h
        displayapp/screens/InfiniPaint.h
        displayapp/screens/StopWatch.h
        displayapp/screens/Paddle.h
        displayapp/screens/BatteryIcon.h
        displayapp/screens/BleIcon.h
        displayapp/screens/NotificationIcon.h
        displayapp/screens/SystemInfo.h
        displayapp/screens/ScreenList.h
        displayapp/screens/Label.h
        displayapp/screens/FirmwareUpdate.h
        displayapp/screens/FirmwareValidation.h
        displayapp/screens/ApplicationList.h
        displayapp/screens/CheckboxList.h
        displayapp/Apps.h
        displayapp/WatchFaces.h
        displayapp/screens/Notifications.h
        displayapp/screens/HeartRate.h
        displayapp/screens/Metronome.h
        displayapp/screens/Motion.h
        displayapp/screens/Timer.h
        displayapp/screens/Alarm.h
        displayapp/Colors.h
        displayapp/widgets/Counter.h
        displayapp/widgets/PageIndicator.h
        displayapp/widgets/DotIndicator.h
        displayapp/widgets/StatusIcons.h
        drivers/St7789.h
        drivers/SpiNorFlash.h
        drivers/SpiMaster.h
        drivers/Spi.h
        drivers/Watchdog.h
        drivers/InternalFlash.h
        drivers/Hrs3300.h
        drivers/PinMap.h
        drivers/Bma421.h
        drivers/Bma421_C/bma4.c
        drivers/Bma421_C/bma423.c
        components/battery/BatteryController.h
        components/ble/BleController.h
        components/ble/NotificationManager.h
        components/datetime/DateTimeController.h
        components/brightness/BrightnessController.h
        components/motion/MotionController.h
        components/firmwarevalidator/FirmwareValidator.h
        components/ble/BleController.h
        components/ble/NotificationManager.h
        components/ble/NimbleController.h
        components/ble/DeviceInformationService.h
        components/ble/CurrentTimeClient.h
        components/ble/AlertNotificationClient.h
        components/ble/DfuService.h
        components/firmwarevalidator/FirmwareValidator.h
        components/ble/BatteryInformationService.h
        components/ble/FSService.h
        components/ble/ImmediateAlertService.h
        components/ble/ServiceDiscovery.h
        components/ble/BleClient.h
        components/ble/HeartRateService.h
        components/ble/MotionService.h
        components/ble/weather/WeatherService.h
        components/settings/Settings.h
        components/timer/Timer.h
        components/alarm/AlarmController.h
        drivers/Cst816s.h
        FreeRTOS/portmacro.h
        FreeRTOS/portmacro_cmsis.h
        displayapp/LittleVgl.h
        displayapp/InfiniTimeTheme.h
        systemtask/SystemTask.h
        systemtask/SystemMonitor.h
        displayapp/screens/Symbols.h
        drivers/TwiMaster.h
        heartratetask/HeartRateTask.h
        components/heartrate/Ppg.h
        components/heartrate/HeartRateController.h
        libs/arduinoFFT/src/arduinoFFT.h
        libs/arduinoFFT/src/defs.h
        libs/arduinoFFT/src/types.h
        components/motor/MotorController.h
        buttonhandler/ButtonHandler.h
        touchhandler/TouchHandler.h
        utility/Math.h
        )

include_directories(
        ${CMAKE_BINARY_DIR}/src # include generated files like Version.h
        .
        ../
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

# Note: Only use this for debugging
# Derive the low frequency clock from the main clock (SYNT)
# add_definitions(-DCLOCK_CONFIG_LF_SRC=2)

# Target hardware configuration options
add_definitions(-DTARGET_DEVICE_${TARGET_DEVICE})
add_definitions(-DTARGET_DEVICE_NAME="${TARGET_DEVICE}")
if(TARGET_DEVICE STREQUAL "PINETIME")
  add_definitions(-DDRIVER_PINMAP_PINETIME)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY-TFK5") # P8a
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY-TIN5") # P8a variant 2
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=1) # XTAL
elseif(TARGET_DEVICE STREQUAL "MOY-TON5") # P8b
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=0) # RC
  add_definitions(-DMYNEWT_VAL_BLE_LL_SCA=500)
  add_definitions(-DCLOCK_CONFIG_LF_CAL_ENABLED=1)
elseif(TARGET_DEVICE STREQUAL "MOY-UNK") # P8b mirrored
  add_definitions(-DDRIVER_PINMAP_P8)
  add_definitions(-DCLOCK_CONFIG_LF_SRC=0) # RC
  add_definitions(-DMYNEWT_VAL_BLE_LL_SCA=500)
  add_definitions(-DCLOCK_CONFIG_LF_CAL_ENABLED=1)
  add_definitions(-DDRIVER_DISPLAY_MIRROR)
else()
  message(FATAL_ERROR "Invalid TARGET_DEVICE")
endif()

# Debug configuration
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-DDEBUG)
  add_definitions(-DDEBUG_NRF_USER)

  # NRF SDK Logging
  add_definitions(-DNRF_LOG_ENABLED=1)
  # add_definitions(-DNRF_LOG_BACKEND_RTT_ENABLED=1)
  # add_definitions(-DNRF_LOG_BACKEND_SERIAL_USES_RTT=1)

  # NRF SDK individual modules logging
  # add_definitions(-DCLOCK_CONFIG_LOG_ENABLED=1)
  # add_definitions(-DCLOCK_CONFIG_LOG_LEVEL=4)
  # add_definitions(-DRTC_CONFIG_LOG_ENABLED=1)
  # add_definitions(-DRTC_CONFIG_LOG_LEVEL=4)
  
  # Nimble Logging
  add_definitions(-DMYNEWT_VAL_NEWT_FEATURE_LOGCFG=1)
  # add_definitions(-DMYNEWT_VAL_LOG_LEVEL=0)
  # add_definitions(-DMYNEWT_VAL_BLE_HS_LOG_LVL=0)
endif()

add_subdirectory(displayapp/fonts)
target_compile_options(infinitime_fonts PUBLIC
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# NRF SDK
add_library(nrf-sdk STATIC ${SDK_SOURCE_FILES})
target_include_directories(nrf-sdk SYSTEM PUBLIC . ../)
target_include_directories(nrf-sdk SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(nrf-sdk PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        -O3
        )

# NimBLE
add_library(nimble STATIC ${NIMBLE_SRC} ${TINYCRYPT_SRC})
target_include_directories(nimble SYSTEM PUBLIC . ../)
target_include_directories(nimble SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(nimble PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# lvgl
add_library(lvgl STATIC ${LVGL_SRC})
target_include_directories(lvgl SYSTEM PUBLIC . ../)
target_include_directories(lvgl SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(lvgl PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# QCBOR
add_library(QCBOR STATIC ${QCBOR_SRC})
target_include_directories(QCBOR SYSTEM PUBLIC libs/QCBOR/inc)
# This is required with the current configuration
target_compile_definitions(QCBOR PUBLIC QCBOR_DISABLE_FLOAT_HW_USE)
# These are for space-saving
target_compile_definitions(QCBOR PUBLIC QCBOR_DISABLE_PREFERRED_FLOAT)
target_compile_definitions(QCBOR PUBLIC QCBOR_DISABLE_EXP_AND_MANTISSA)
target_compile_definitions(QCBOR PUBLIC QCBOR_DISABLE_INDEFINITE_LENGTH_STRINGS)
#target_compile_definitions(QCBOR PUBLIC QCBOR_DISABLE_INDEFINITE_LENGTH_ARRAYS)
target_compile_definitions(QCBOR PUBLIC QCBOR_DISABLE_UNCOMMON_TAGS)
target_compile_definitions(QCBOR PUBLIC USEFULBUF_CONFIG_LITTLE_ENDIAN)
set_target_properties(QCBOR PROPERTIES LINKER_LANGUAGE C)
target_compile_options(QCBOR PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        -O3
        )

# LITTLEFS_SRC
add_library(littlefs STATIC ${LITTLEFS_SRC})
target_include_directories(littlefs SYSTEM PUBLIC . ../)
target_include_directories(littlefs SYSTEM PUBLIC ${INCLUDES_FROM_LIBS})
target_compile_options(littlefs PRIVATE
        ${COMMON_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

# Build autonomous binary (without support for bootloader)
set(EXECUTABLE_NAME "pinetime-app")
set(EXECUTABLE_FILE_NAME ${EXECUTABLE_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/gcc_nrf52.ld")
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_FILE_NAME})
target_link_libraries(${EXECUTABLE_NAME} nimble nrf-sdk lvgl littlefs QCBOR infinitime_fonts)
target_compile_options(${EXECUTABLE_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_FILE_NAME}.out "${EXECUTABLE_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_FILE_NAME}.out "${EXECUTABLE_FILE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_FILE_NAME}")

if(BUILD_RESOURCES)
  add_dependencies(${EXECUTABLE_NAME} GenerateResources)
endif()

# Build binary intended to be used by bootloader
set(EXECUTABLE_MCUBOOT_NAME "pinetime-mcuboot-app")
set(EXECUTABLE_MCUBOOT_FILE_NAME ${EXECUTABLE_MCUBOOT_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_FILE_NAME_HEX ${EXECUTABLE_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.hex)
set(IMAGE_MCUBOOT_FILE_NAME_BIN ${EXECUTABLE_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.bin)
set(DFU_MCUBOOT_FILE_NAME ${EXECUTABLE_MCUBOOT_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
set(NRF5_LINKER_SCRIPT_MCUBOOT "${CMAKE_SOURCE_DIR}/gcc_nrf52-mcuboot.ld")
add_executable(${EXECUTABLE_MCUBOOT_NAME} ${SOURCE_FILES})
target_link_libraries(${EXECUTABLE_MCUBOOT_NAME} nimble nrf-sdk lvgl littlefs QCBOR infinitime_fonts)
set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_MCUBOOT_FILE_NAME})
target_compile_options(${EXECUTABLE_MCUBOOT_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_MCUBOOT_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_MCUBOOT_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_MCUBOOT_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_FILE_NAME}.hex"
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_FILE_NAME}.hex ${IMAGE_MCUBOOT_FILE_NAME_HEX}
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_FILE_NAME}.bin ${IMAGE_MCUBOOT_FILE_NAME_BIN}
        COMMENT "post build steps for ${EXECUTABLE_MCUBOOT_FILE_NAME}"
        )

if(BUILD_RESOURCES)
  add_dependencies(${EXECUTABLE_MCUBOOT_NAME} GenerateResources)
endif()

if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_NAME}
          POST_BUILD
          COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_MCUBOOT_FILE_NAME_HEX} ${DFU_MCUBOOT_FILE_NAME}
          COMMENT "post build (DFU) steps for ${EXECUTABLE_MCUBOOT_FILE_NAME}"
          )
endif()

# InfiniTime recovery firmware (autonomous)
set(EXECUTABLE_RECOVERY_NAME "pinetime-recovery")
set(EXECUTABLE_RECOVERY_FILE_NAME ${EXECUTABLE_RECOVERY_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
add_executable(${EXECUTABLE_RECOVERY_NAME} ${RECOVERY_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_RECOVERY_NAME} nimble nrf-sdk littlefs QCBOR infinitime_fonts)
set_target_properties(${EXECUTABLE_RECOVERY_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_RECOVERY_FILE_NAME})
target_compile_definitions(${EXECUTABLE_RECOVERY_NAME} PUBLIC "PINETIME_IS_RECOVERY")
target_compile_options(${EXECUTABLE_RECOVERY_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_RECOVERY_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_RECOVERY_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_RECOVERY_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_RECOVERY_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_RECOVERY_FILE_NAME}.out "${EXECUTABLE_RECOVERY_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_RECOVERY_FILE_NAME}.out "${EXECUTABLE_RECOVERY_FILE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_RECOVERY_FILE_NAME}"
        )

# InfiniTime recovery firmware (mcuboot)
set(EXECUTABLE_RECOVERY_MCUBOOT_NAME "pinetime-mcuboot-recovery")
set(EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_RECOVERY_MCUBOOT_FILE_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME}.hex)
set(DFU_RECOVERY_MCUBOOT_FILE_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
add_executable(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} ${RECOVERY_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} nimble nrf-sdk littlefs QCBOR infinitime_fonts)
set_target_properties(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME})
target_compile_definitions(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PUBLIC "PINETIME_IS_RECOVERY")
target_compile_options(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )

set_target_properties(${EXECUTABLE_RECOVERY_MCUBOOT_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_GRAPHICS_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.out "${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.hex"
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}.hex ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX}
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O binary ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX} "${IMAGE_RECOVERY_MCUBOOT_FILE_NAME}.bin"
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/bin2c.py ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME}.bin recoveryImage > recoveryImage.h
        COMMENT "post build steps for ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}"
        )

if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_RECOVERY_MCUBOOT_NAME}
          POST_BUILD
          COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_RECOVERY_MCUBOOT_FILE_NAME_HEX} ${DFU_RECOVERY_MCUBOOT_FILE_NAME}
          COMMENT "post build (DFU) steps for ${EXECUTABLE_RECOVERY_MCUBOOT_FILE_NAME}"
          )
endif()

# Build binary that writes the recovery image into the SPI flash memory
set(EXECUTABLE_RECOVERYLOADER_NAME "pinetime-recovery-loader")
set(EXECUTABLE_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_RECOVERYLOADER_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
add_executable(${EXECUTABLE_RECOVERYLOADER_NAME} ${RECOVERYLOADER_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_RECOVERYLOADER_NAME} nrf-sdk QCBOR infinitime_fonts)
set_target_properties(${EXECUTABLE_RECOVERYLOADER_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_RECOVERYLOADER_FILE_NAME})
target_compile_options(${EXECUTABLE_RECOVERYLOADER_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )
target_include_directories(${EXECUTABLE_RECOVERYLOADER_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
        )
add_dependencies(${EXECUTABLE_RECOVERYLOADER_NAME} ${EXECUTABLE_RECOVERY_MCUBOOT_NAME})

set_target_properties(${EXECUTABLE_RECOVERYLOADER_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_RECOVERYLOADER_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_RECOVERYLOADER_FILE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_RECOVERYLOADER_FILE_NAME}"
        )

# Build binary that writes the recovery image (MCUBoot version)
set(EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME "pinetime-mcuboot-recovery-loader")
set(EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}-image-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH})
set(IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.hex)
set(DFU_MCUBOOT_RECOVERYLOADER_FILE_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}-dfu-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip)
add_executable(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} ${RECOVERYLOADER_SOURCE_FILES})
target_link_libraries(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} nrf-sdk QCBOR infinitime_fonts)
set_target_properties(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME})
target_compile_options(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PUBLIC
        ${COMMON_FLAGS}
        ${WARNING_FLAGS}
        $<$<CONFIG:DEBUG>: ${DEBUG_FLAGS}>
        $<$<CONFIG:RELEASE>: ${RELEASE_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>: ${CXX_FLAGS}>
        $<$<COMPILE_LANGUAGE:ASM>: ${ASM_FLAGS}>
        )
target_include_directories(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
        )
add_dependencies(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} ${EXECUTABLE_RECOVERY_MCUBOOT_NAME})

set_target_properties(${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME} PROPERTIES
        SUFFIX ".out"
        LINK_FLAGS "-mthumb -mabi=aapcs -std=gnu++98 -std=c99 -L ${NRF5_SDK_PATH}/modules/nrfx/mdk -T${NRF5_LINKER_SCRIPT_MCUBOOT} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections -Wl,--print-memory-usage --specs=nano.specs -lc -lnosys -lm -Wl,-Map=${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.map"
        )

add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.out
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.out "${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.hex"
        COMMAND ${CMAKE_SOURCE_DIR}/tools/mcuboot/imgtool.py create --align 1 --version 1.0.0 --header-size 32 --slot-size 475136 --pad-header ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.hex ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX}
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O binary ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX} "${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.bin"
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/bin2c.py ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME}.bin recoveryLoaderImage > recoveryLoaderImage.h
        COMMENT "post build steps for ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}"
        )

if(BUILD_DFU)
  add_custom_command(TARGET ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_NAME}
          POST_BUILD
          COMMAND adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ${IMAGE_MCUBOOT_RECOVERYLOADER_FILE_NAME_HEX} ${DFU_MCUBOOT_RECOVERYLOADER_FILE_NAME}
          COMMENT "post build (DFU) steps for ${EXECUTABLE_MCUBOOT_RECOVERYLOADER_FILE_NAME}"
          )
endif()

if(BUILD_RESOURCES)
  add_subdirectory(resources)
endif()

