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


include_directories(.)
include_directories(libs/)

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
  libs/lvgl/src/lv_font/lv_font_roboto_16.c
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

  )

list(APPEND SOURCE_FILES
  Logging/NrfLogger.cpp
  BlinkApp/BlinkApp.cpp
  DisplayApp/DisplayApp.cpp
  DisplayApp/Screens/Screen.cpp
  DisplayApp/Screens/Clock.cpp
  DisplayApp/Screens/Message.cpp
  DisplayApp/Screens/Tile.cpp
#  DisplayApp/Screens/Tab.cpp
  main.cpp
  drivers/St7789.cpp
  drivers/SpiMaster.cpp
#  Components/Gfx/Gfx.cpp
  BLE/BleManager.c
  Components/Battery/BatteryController.cpp
  Components/Ble/BleController.cpp
  Components/DateTime/DateTimeController.cpp
  drivers/Cst816s.cpp
  FreeRTOS/port.c
  FreeRTOS/port_cmsis_systick.c
  FreeRTOS/port_cmsis.c
  ${LVGL_SRC}

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
  DisplayApp/Screens/Screen.h
  DisplayApp/Screens/Clock.h
  DisplayApp/Screens/Message.h
  DisplayApp/Screens/Tile.h
#  DisplayApp/Screens/Tab.h
  drivers/St7789.h
  drivers/SpiMaster.h
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

  DisplayApp/LittleVgl.h

  SystemTask/SystemTask.h
)

include_directories(
  FreeRTOS/
  libs/date/includes
)

link_directories(
  ../
)

nRF5x_addExecutable(pinetime-app "${SOURCE_FILES}")
