cmake_minimum_required(VERSION 3.10)

set(FONTS jetbrains_mono_42 jetbrains_mono_76 jetbrains_mono_bold_20
   jetbrains_mono_extrabold_compressed lv_font_navi_80 lv_font_sys_48
   open_sans_light)
find_program(LV_FONT_CONV "lv_font_conv" NO_CACHE REQUIRED)
configure_file(${CMAKE_CURRENT_LIST_DIR}/jetbrains_mono_bold_20.c_zero.patch
   displayapp/fonts/jetbrains_mono_bold_20.c_zero.patch COPYONLY)
foreach(FONT ${FONTS})
   set_source_files_properties(displayapp/fonts/${FONT}.c
      PROPERTIES GENERATED TRUE)

   add_custom_command(OUTPUT displayapp/fonts/${FONT}.c
      COMMAND python ${CMAKE_CURRENT_LIST_DIR}/generate.py
      -f ${FONT} ${CMAKE_CURRENT_LIST_DIR}/fonts.json
      DEPENDS ${CMAKE_CURRENT_LIST_DIR}/fonts.json
      WORKING_DIRECTORY displayapp/fonts
      )
endforeach()
