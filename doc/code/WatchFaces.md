# Add Watchfaces 

This page will help you to add a new Watch Face to InfiniTime.

## Theory

This example uses the existing Digital Watch Face, and copies it to a new watch face as a starting point.

## Files 

- displayapp/UserApps.h
- displayapp/apps/Apps.h.in
- displayapp/apps/CMakeLists.txt
- displayapp/screens/WatchFaceNew.cpp 
- displayapp/screens/WatchFaceNew.h 
- CMakeLists.txt

### Branch and Copy 

Use git to fork and get a local copy (commands not shown). 

Then with all the submodules updated add a branch and copy the watch face files

```bash
git submodule update --init
git switch -c Watchface_New
cp ./src/displayapp/screens/WatchFaceDigital.cpp ./src/displayapp/screens/WatchFaceNew.cpp
cp ./src/displayapp/screens/WatchFaceDigital.h ./src/displayapp/screens/WatchFaceNew.h
```

### Amend Files

Use sed to update `WatchFaceDigital` in the newly copied files with `WatchFaceNew` as well as update the `WatchFace::Digital` type to `WatchFace::NewType`.

```bash
sed -i 's/WatchFaceDigital/WatchFaceNew/' src/displayapp/screens/WatchFaceNew.cpp src/displayapp/screens/WatchFaceNew.h
sed -i 's/WatchFace::Digital/WatchFace::NewType/' src/displayapp/screens/WatchFaceNew.h
```

In UserApps.h add include for the new Watch Face.

./src/displayapp/UserApps.h:

```cpp
...
#include "displayapp/screens/WatchFaceNew.h"
```

In Apps.h.in add the type from WatchFaceNew.h.

./src/displayapp/apps/Apps.h.in:

```cpp
enum class WatchFace : uint8_t {
...
CasioStyleG7710,
NewType,
```

In CMakeLists.txt add the type to the DEFAULT_WATCHFACE_TYPES so that it is included in the firmware.

./src/displayapp/apps/CMakeLists.txt:

```cmake
...
set(DEFAULT_WATCHFACE_TYPES "${DEFAULT_WATCHFACE_TYPES}, WatchFace::CasioStyleG7710")
set(DEFAULT_WATCHFACE_TYPES "${DEFAULT_WATCHFACE_TYPES}, WatchFace::NewType")
```

In CMakeLists.txt include the WatchFaceNew.cpp source file.

./src/CMakeLists.txt:

```cpp
...
displayapp/screens/WatchFacePineTimeStyle.cpp
displayapp/screens/WatchFaceCasioStyleG7710.cpp
displayapp/screens/WatchFaceNew.cpp
```

## Test Copied Watch Face

At this point - assuming you have [InfiniTime simulator](https://github.com/InfiniTimeOrg/InfiniSim) locally installed. It is a good time to test. 

If you already have a `build/CMakeCache.txt` it will prevent your new watch face from being included in the build, so you will need to pass the `--fresh` argument to `cmake` or remove `build/CMakeCache.txt` before building. (The value of DEFAULT_WATCHFACE_TYPES is cached.)

## Amending the Watch Face to Suit 

Start editing and ammended WatchFaceNew.cpp to suit your tastes. To save space on the firmware, use existing fonts. 

## Finish

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app or Watch Face that you want to be included in InfiniTime.
