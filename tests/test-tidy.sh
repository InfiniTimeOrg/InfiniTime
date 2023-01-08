#!/bin/sh

set -e

if [ -z "$GITHUB_BASE_REF" ]
then
  echo "This script is only meant to be run in a GitHub Workflow"
  exit 1
fi

CHANGED_FILES=$(git diff --name-only "$GITHUB_BASE_REF"...HEAD)

for file in $CHANGED_FILES
do
  [ -e "$file" ] || continue
  case "$file" in
  src/libs/*|src/FreeRTOS/*) continue ;;
  *.cpp|*.h)
    echo "::group::$file"
    clang-tidy-14 -p build "$file" || true
    echo "::endgroup::"
  esac
done

exit 0
