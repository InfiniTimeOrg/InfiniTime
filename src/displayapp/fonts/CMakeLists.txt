cmake_minimum_required(VERSION 3.10)

set(FONTS jetbrains_mono_42 jetbrains_mono_76 jetbrains_mono_bold_20
   jetbrains_mono_extrabold_compressed lv_font_navi_80 lv_font_sys_48
   open_sans_light)

foreach(FONT ${FONTS})
   set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/displayapp/fonts/${FONT}.c 
      PROPERTIES GENERATED TRUE)

   add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/displayapp/fonts/${FONT}.c
      COMMAND python generate.py -f ${FONT} fonts.json
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/displayapp/fonts/fonts.json
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/displayapp/fonts
      )
endforeach()
