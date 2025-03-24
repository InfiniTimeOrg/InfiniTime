#!/bin/sh

set -e

if [ -z "$GITHUB_BASE_REF" ]
then
  echo "This script is only meant to be run in a GitHub Workflow"
  exit 1
fi

CHANGED_FILES=$(git diff --name-only "$GITHUB_BASE_REF"...HEAD)

CHANGED=0

for file in $CHANGED_FILES
do
  [ -e "$file" ] || continue
  case "$file" in
  src/libs/*|src/FreeRTOS/*) continue ;;
  *.cpp|*.h)
    echo Checking "$file"
    PATCH="$(basename "$file").patch"
    git clang-format-14 -q --style file --diff "$GITHUB_BASE_REF" "$file" > "$PATCH"
    if [ -s "$PATCH" ]
    then
      printf "\033[31mError:\033[0m Formatting error in %s\n" "$file"
      CHANGED=1
    else
      rm "$PATCH"
    fi
  esac
done

if [ $CHANGED = 1 ]
then
  printf "\033[31mError:\033[0m Issues found. You may use the patches provided as artifacts to format the code."
  exit 1
fi

exit 0
