set(FONTS teko bebas lv_font_dots_40
seven_segments_40 seven_segments_115)

find_program(LV_FONT_CONV "lv_font_conv" NO_CACHE REQUIRED
    HINTS "${CMAKE_SOURCE_DIR}/node_modules/.bin")
message(STATUS "Using ${LV_FONT_CONV} to generate font files")

find_program(LV_IMG_CONV "lv_img_conv.py" NO_CACHE REQUIRED
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Using ${LV_IMG_CONV} to generate font files")

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
   # FindPython3 module introduces with CMake 3.12
   # https://cmake.org/cmake/help/latest/module/FindPython3.html
   set(Python3_FIND_STRATEGY LOCATION) # https://discourse.cmake.org/t/find-package-python3-is-not-finding-the-correct-python/10563
   find_package(Python3 REQUIRED)
else()
   set(Python3_EXECUTABLE "python")
endif()

add_library(compiled_fonts OBJECT)
# add include directory to lvgl headers needed to compile the font files on its own
target_include_directories(compiled_fonts PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../libs")
foreach(FONT ${FONTS})
   add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FONT}.c
      COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-font-c.py
      --lv-font-conv "${LV_FONT_CONV}"
      --font ${FONT} ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )
   add_custom_target(compiled_fonts_${FONT}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${FONT}.c
   )
   target_sources(compiled_fonts PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${FONT}.c")
   add_dependencies(compiled_fonts compiled_fonts_${FONT})
endforeach()


# generate fonts
add_custom_target(GenerateResources
    COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-img.py  --lv-img-conv "${LV_IMG_CONV}" ${CMAKE_CURRENT_SOURCE_DIR}/images.json
    COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-font-bin.py $<TARGET_OBJECTS:compiled_fonts>
    COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-package.py --config  ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json --config  ${CMAKE_CURRENT_SOURCE_DIR}/images.json --obsolete obsolete_files.json --output infinitime-resources-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip
    COMMAND_EXPAND_LISTS
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}

    COMMENT "Generate fonts and images for resource package"
)

add_dependencies(GenerateResources compiled_fonts)
