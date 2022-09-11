
find_program(LV_FONT_CONV "lv_font_conv" NO_CACHE REQUIRED
    HINTS "${CMAKE_SOURCE_DIR}/node_modules/.bin")
message(STATUS "Using ${LV_FONT_CONV} to generate font files")

find_program(LV_IMG_CONV "lv_img_conv" NO_CACHE REQUIRED
    HINTS "${CMAKE_SOURCE_DIR}/node_modules/.bin")
message(STATUS "Using ${LV_IMG_CONV} to generate font files")

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
   # FindPython3 module introduces with CMake 3.12
   # https://cmake.org/cmake/help/latest/module/FindPython3.html
   find_package(Python3 REQUIRED)
else()
   set(Python3_EXECUTABLE "python")
endif()

# generate fonts
add_custom_target(GenerateResources
    COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-fonts.py  --lv-font-conv "${LV_FONT_CONV}" ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json
    COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-img.py  --lv-img-conv "${LV_IMG_CONV}" ${CMAKE_CURRENT_SOURCE_DIR}/images.json
    COMMAND "${Python3_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate-package.py --config  ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json --config  ${CMAKE_CURRENT_SOURCE_DIR}/images.json --obsolete obsolete_files.json --output infinitime-resources-${pinetime_VERSION_MAJOR}.${pinetime_VERSION_MINOR}.${pinetime_VERSION_PATCH}.zip
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fonts.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}

    COMMENT "Generate fonts and images for resource package"
)

