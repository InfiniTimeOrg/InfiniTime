# load git variables for build date and time
find_package(Git)
if (GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} status
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GIT_STATUS_ERROR
    OUTPUT_QUIET
    ERROR_QUIET
  )
else()
  set(GIT_STATUS_ERROR YES)
endif()
if (GIT_STATUS_ERROR)
  message(STATUS "Git or git archive was not found")
  set(PROJECT_GIT_REF "unknown")
  set(PROJECT_GIT_IS_TAG "NO")
  set(PROJECT_GIT_COMMIT_HASH "dirty")
  set(PROJECT_GIT_REPO_IS_CLEAN "NO")
  string(TIMESTAMP PROJECT_GIT_TIMESTAMP "%Y-%m-%d %H:%M:%S +0000" UTC)
else()
  # VARIABLE: COMMIT_HASH
  if(DEFINED ENV{GITLAB_CI})
    set(PROJECT_GIT_COMMIT_HASH $ENV{CI_COMMIT_SHORT_SHA})
  elseif(DEFINED ENV{CI})
    set(PROJECT_GIT_COMMIT_HASH $ENV{GITHUB_SHA})
  else()
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
      COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE PROJECT_GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
  # VARIABLE: LATEST_TAG
  # Get the last tag (if any)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_LAST_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  # VARIABLE: IS_TAG and TAG
  if(DEFINED ENV{GITLAB_CI})
      if(DEFINED ENV{CI_COMMIT_TAG})
        set(PROJECT_GIT_IS_TAG "YES")
        set(PROJECT_GIT_REF $ENV{CI_COMMIT_TAG})
      else()
        set(PROJECT_GIT_IS_TAG "NO")
        set(PROJECT_GIT_REF $ENV{CI_COMMIT_BRANCH})
      endif()
  elseif(DEFINED ENV{CI})
    if($ENV{GITHUB_REF_TYPE} STREQUAL "tag")
      set(PROJECT_GIT_IS_TAG "YES")
      set(PROJECT_GIT_REF $ENV{GITHUB_REF_NAME})
    else()
      set(PROJECT_GIT_IS_TAG "NO")
      set(PROJECT_GIT_REF $ENV{GITHUB_REF_NAME})
    endif()
  else()
    # VARIABLE: COMMIT_COUNT
    # count the number of commits since last tag or since the beginning
    if ("${PROJECT_GIT_LAST_TAG}" STREQUAL "")
      set(git_last_tag_flag "")
    else()
      set(git_last_tag_flag "^${PROJECT_GIT_LAST_TAG}")
    endif()
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD ${git_last_tag_flag}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE PROJECT_GIT_COMMIT_COUNT
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (PROJECT_GIT_COMMIT_COUNT EQUAL "0")
      set(PROJECT_GIT_IS_TAG "YES")
      set(PROJECT_GIT_REF "${PROJECT_GIT_LAST_TAG}")
    else()
      set(PROJECT_GIT_IS_TAG "NO")
      # extract the name of the branch the last commit has been made on, so this also works
      # in detached head state (which is the default on CI)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} name-rev --name-only HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECT_GIT_REF
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      STRING(REGEX REPLACE "^remotes/origin/" "" PROJECT_GIT_REF ${PROJECT_GIT_REF})
    endif()
  endif()
  # VARIABLE: REPO_IS_CLEAN
  execute_process(
    COMMAND ${GIT_EXECUTABLE} diff --no-ext-diff --quiet --exit-code
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE repo_is_clean_flag
    OUTPUT_QUIET
    ERROR_QUIET
  )
  if (repo_is_clean_flag EQUAL "0")
    set(PROJECT_GIT_REPO_IS_CLEAN "YES")
  else()
    set(PROJECT_GIT_REPO_IS_CLEAN "NO")
  endif()
  if(DEFINED ENV{GITLAB_CI} OR DEFINED ENV{CI})
    if(NOT ${PROJECT_GIT_REPO_IS_CLEAN})
      message(AUTHOR_WARNING "Detected a repo with modifications in a CI build.\nSomething is rotten in the state of Denmark.")
    endif()
  endif()
  # VARIABLE: COMMIT_TIMESTAMP
  if (PROJECT_GIT_REPO_IS_CLEAN)
    if (DEFINED ENV{TZ})
      set(tz_env_backup ENV{TZ})
    else()
      set(tz_env_backup "")
    endif()
    set(ENV{TZ} "UTC")  # force git timestamp to UTC as local format
    # if the repo is clean use the latest commit time in UTC
    execute_process(
      COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=iso-local
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE git_date_successful
      OUTPUT_VARIABLE PROJECT_GIT_TIMESTAMP
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT git_date_successful EQUAL "0")
      # if the repo is clean use the latest commit time as is
      execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=iso
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE git_date_successful
        OUTPUT_VARIABLE PROJECT_GIT_TIMESTAMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    endif()
    if (NOT git_date_successful EQUAL "0")
      message(ERROR "Failed to read last commit timestamp from Git in either timezone.")
    endif()
    unset(git_date_successful)

    if (NOT tz_env_backup STREQUAL "")
      set(ENV{TZ} ${tz_env_backup})
    endif()
    unset(tz_env_backup)
  else()
    # if the repo is dirty use the current date and time
    string(TIMESTAMP PROJECT_GIT_TIMESTAMP "%Y-%m-%d %H:%M:%S +0000" UTC)
  endif()
endif()

# be sure the commit sha is at most 8 characters long
string(SUBSTRING "${PROJECT_GIT_COMMIT_HASH}" 0 8 PROJECT_GIT_COMMIT_HASH)
# extract date and time to use instead of the compiler macros __DATE__ and __TIME__
string(SUBSTRING "${PROJECT_GIT_TIMESTAMP}" 0 10 PROJECT_GIT_TIMESTAMP_DATE)
string(SUBSTRING "${PROJECT_GIT_TIMESTAMP}" 11 8 PROJECT_GIT_TIMESTAMP_TIME)

message("GIT VARIABLES
-------------
    * PROJECT_GIT_REF:            ${PROJECT_GIT_REF}
    * PROJECT_GIT_IS_TAG:         ${PROJECT_GIT_IS_TAG}
    * PROJECT_GIT_COMMIT_HASH:    ${PROJECT_GIT_COMMIT_HASH}
    * PROJECT_GIT_REPO_IS_CLEAN:  ${PROJECT_GIT_REPO_IS_CLEAN}
    * PROJECT_GIT_TIMESTAMP:      ${PROJECT_GIT_TIMESTAMP}
    * PROJECT_GIT_TIMESTAMP_DATE: ${PROJECT_GIT_TIMESTAMP_DATE}
    * PROJECT_GIT_TIMESTAMP_TIME: ${PROJECT_GIT_TIMESTAMP_TIME}
")
