#!/bin/bash
(return 0 2>/dev/null) && SOURCED="true" || SOURCED="false"
export LC_ALL=C.UTF-8
export LANG=C.UTF-8
set -e

# Default locations if the var isn't already set
export SOURCES_DIR="${SOURCES_DIR:=/sources}"

main() {
  local target="$1"

  case "$1" in
    "build-simulator")
      cd $SOURCES_DIR
      BuildSimulator $2 $3
      BUILD_RESULT=$?
      ;;
    "test-format")
      cd $SOURCES_DIR
      TestFormat $2
      BUILD_RESULT=$?
      ;;
    *)
      echo "Usage: $0 build-simulator [repo branch]"
      echo "Usage: $0 test-format [commit]"
      ;;
  esac

  return $BUILD_RESULT
}

BuildSimulator() {
  if [ "$1" != "" ] && [ -e "$SOURCES_DIR/InfiniSim" ]
  then
    echo -e "\033[31mRepo specified and InfiniSim subfolder exists.\033[0m"
    return 1
  elif [ "$1" != "" ] && [ "$2" == "" ]
  then
    echo -e "\033[31mRepo specified, branch not specified.\033[0m"
    return 1
  elif [ "$2" != "" ]
  then
    GetSimulator $1 $2
  elif [ ! -d "$SOURCES_DIR/InfiniSim" ]
  then
    GetSimulator https://github.com/InfiniTimeOrg/InfiniSim.git main
  else
    echo -e "\033[33mUsing InfiniSim from InfiniSim subfolder.\033[0m"
  fi
  cmake -G Ninja -S InfiniSim -B build_lv_sim -DInfiniTime_DIR="${PWD}"
  BUILD_RESULT=$?
  [ "$BUILD_RESULT" == "0" ] || return $BUILD_RESULT
  cmake --build build_lv_sim --clean-first
  BUILD_RESULT=$?
  [ "$BUILD_RESULT" == "0" ] && echo -e "\033[32mSIMULATOR BUILD SUCCEEDED! \033[0m"
  cd $SOURCES_DIR
  return $BUILD_RESULT
}

GetSimulator() {
  [ ! -e "$SOURCES_DIR/InfiniSim" ] || return 1
  trap "rm $SOURCES_DIR/InfiniSim" EXIT
  ln -s /tmp/InfiniSim $SOURCES_DIR/InfiniSim
  echo -e "\033[33mDownloading InfiniSim Git repo from: $1 $2 \033[0m"
  git -C /tmp clone $1 /tmp/InfiniSim --depth 1 --branch $2
  git -C /tmp/InfiniSim submodule update --init lv_drivers
}

TestFormat() {
  CMPBASE=$1
  if [ "$CMPBASE" == "" ]
  then
    if [ "$(git diff --name-only upstream/main...HEAD)" != "" ]
    then
      CMPBASE=upstream/main
    elif [ "$(git diff --name-only origin/main...HEAD)" != "" ]
    then
      CMPBASE=origin/main
    else
      CMPBASE=main
    fi
  fi

  echo -e "\033[33mComparing against: $CMPBASE \033[0m"
  CHANGED_FILES=$(git diff --name-only $CMPBASE...HEAD)
  CHANGED=0

  for file in $CHANGED_FILES
  do
    [ -e "$file" ] || continue
    case "$file" in
    src/libs/*|src/FreeRTOS/*) continue ;;
    *.cpp|*.h)
      echo Checking "$file"
      PATCH="$(basename "$file").patch"
      if [ "`git clang-format-14 -q --style file --diff "$CMPBASE" "$file" | wc -c`" != "0" ]
      then
        printf "\033[94m--------------------------------------------------\033[0m\n"
        printf "\033[31mError:\033[0m Formatting error in %s\n" "$file"
        CHANGED=1
        git clang-format-14 --style file --diff "$CMPBASE" "$file"
        printf "\033[94m--------------------------------------------------\033[0m\n"
      fi
    esac
  done

  if [ $CHANGED = 1 ]
  then
    return 1
  else
    printf "\033[32mNO FORMAT ERRORS DETECTED! \033[0m\n"
    return 0
  fi
}

if [ $SOURCED = "false" ]; then
  # It is important to return exit code of main
  # To be future-proof, this is handled explicitely
  main "$@"
  BUILD_RESULT=$?
  exit $BUILD_RESULT
else
  echo "Sourced!"
fi
