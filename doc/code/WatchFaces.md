# Add Watchfaces 

This page will help you to add a new Watch Face to InfiniTime.

## Theory

This example uses the existing Digital Watch Face, copies this to a new watch face as a starting point to design a new additional watch face.

## Files 

- displayapp/UserApps.h
- displayapp/apps/Apps.h.in
- displayapp/apps/CMakeLists.txt
- displayapp/screens/WatchFaceNew.cpp 
- displayapp/screens/WatchFaceNew.h 
- components/settings/Settings.h
- CMakeLists.txt

### Check out and Branch and Copy 

Use git to fork and get a local copy (commands not shown). 

Then with all the submodules updated add a branch and copy the watch face files

```bash
git submodule update --init
git checkout -b Watchface_New
cp ./src/displayapp/screens/WatchFaceDigital.cpp ./src/displayapp/screens/WatchFaceNew.cpp
cp ./src/displayapp/screens/WatchFaceDigital.h ./src/displayapp/screens/WatchFaceNew.h
```

### Amend Files

In UserApps.h add include for the new Watch Face.

./src/displayapp/UserApps.h:

```cpp

#include "displayapp/screens/WatchFaceTerminal.h"
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

In CMakeLists.txt add the type to the DEFAULT_WATCHFACE_TYPES.

./src/displayapp/apps/CMakeLists.txt:

```cmake

...
set(DEFAULT_WATCHFACE_TYPES "${DEFAULT_WATCHFACE_TYPES}, WatchFace::CasioStyleG7710")
set(DEFAULT_WATCHFACE_TYPES "${DEFAULT_WATCHFACE_TYPES}, WatchFace::NewType")
```

To make the new watchface the default.
In Settings.h change the default value.

./components/settings/Settings.h

```cpp

Pinetime::Applications::WatchFace watchFace = Pinetime::Applications::WatchFace::NewType;
```

In CMakeLists.txt include the WatchFaceNew.cpp source file.

./src/CMakeLists.txt:

```cpp

...
displayapp/screens/WatchFacePineTimeStyle.cpp
displayapp/screens/WatchFaceCasioStyleG7710.cpp
displayapp/screens/WatchFaceNew.cpp
```

Then finally amened WatchFaceNew.cpp and .h with the new names rather than the old WatchFaceDigital

./src/displayapp/screens/WatchFaceNew.cpp:

```cpp
#include "displayapp/screens/WatchFaceNew.h
...
WatchFaceNew::WatchFaceNew(Controllers::DateTime& dateTimeController
...
WatchFaceNew::~WatchFaceNew() {
...
void WatchFaceNew::Refresh() {
```

./src/displayapp/screens/WatchFaceNew.h:

```cpp
...
class WatchFaceNew : public Screen {
...
WatchFaceNew(Controllers::DateTime& dateTimeController,
...
 ~WatchFaceNew() override;
...
struct WatchFaceTraits<WatchFace::NewType> {
      static constexpr WatchFace watchFace = WatchFace::NewType;
...
return new Screens::WatchFaceNew(controllers.dateTimeController,
```

## Test Copied Watch Face

At this point - assuming you have [InfiniTime simulator](https://github.com/InfiniTimeOrg/InfiniSim) locally installed. It is a good time to test. 

If you already have a `build/CMakeCache.txt` it will prevent your new watch face from being included in the build, so you will need to use the `--fresh` option to `cmake` or remove it before building. (The value of DEFAULT_WATCHFACE_TYPES is cached.)

## Amending the Watch Face to Suit 

Start editing and ammended WatchFaceNew.cpp to suit your tastes. To save space on the firmware, use existing fonts. 

## Finish

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app or Watch Face that you want to be included in InfiniTime.
