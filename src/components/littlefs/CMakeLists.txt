
include(FetchContent)
set(FETCHCONTENT_QUIET OFF) # enable GIT_PROGRESS output

FetchContent_Declare(project_littlefs
    GIT_REPOSITORY https://github.com/littlefs-project/littlefs.git
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)

# littlefs Pull
#-------------------------------------------------------------------------------
if(NOT project_littlefs_POPULATED)
    FetchContent_Populate(project_littlefs)
endif()

add_library(littlefs
    ${project_littlefs_SOURCE_DIR}/lfs_util.c
    ${project_littlefs_SOURCE_DIR}/lfs.c
)
target_include_directories(littlefs
    PUBLIC
        ${project_littlefs_SOURCE_DIR}/
)
target_link_libraries(littlefs littlefs-target)

add_library(littlefs-target
    LittleFs.cpp
)
target_include_directories(littlefs-target
    PUBLIC
        #FIXME refactor source into actual components
        ../../drivers/
)
target_link_libraries(littlefs-target littlefs)
