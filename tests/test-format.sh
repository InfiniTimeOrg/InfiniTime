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
  src/libs/*) continue ;;
  *.cpp|*.h)
    echo Checking "$file"
    clang-format -i "$file"
    if ! git diff --quiet
    then
      printf "\033[31mError:\033[0m Formatting error in %s\n" "$file"
      CHANGED=1
      git add "$file"
      git commit -q -m "Apply clang-format to $(basename "$file")"
      printf "Creating patch "
      git format-patch HEAD~
    fi
  esac
done

if [ $CHANGED = 1 ]
then
  printf "\033[31mError:\033[0m Issues found. You may use the patches provided as artifacts to format the code."
  exit 1
fi

exit 0
