if(DEFINED ENABLE_USERAPPS)
    set(USERAPP_TYPES ${ENABLE_USERAPPS} CACHE STRING "List of user apps to build into the firmware")
else ()
    set(USERAPP_TYPES "Apps::Navigation, Apps::StopWatch, Apps::Alarm, Apps::Timer, Apps::Steps, Apps::HeartRate, Apps::Music, Apps::Paint, Apps::Paddle, Apps::Twos, Apps::Metronome" CACHE STRING "List of user apps to build into the firmware")
endif ()

add_library(infinitime_apps INTERFACE)
target_sources(infinitime_apps INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/Apps.h")
target_include_directories(infinitime_apps INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/")

# Generate the list of user apps to be compiled into the firmware
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Apps.h.in ${CMAKE_CURRENT_BINARY_DIR}/Apps.h)
