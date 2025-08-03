# Add Watchfaces 

This page will help you to add a new Watch Face to InfiniTime.

## Theory

This example uses the existing Digital Watch Face, copies this to a new watch face as a starting point to design a new additional watch face.

## Files 

- WatchFaceNew.cpp 
- WatchFaceNew.h 
- Clock.cpp
- Clock.h
- SettingWatchFace.h
- CMakeLists.cpp

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

Include reference to the new Watch Face

./src/displayapp/screens/Clock.cpp:

```cpp

#include "displayapp/screens/WatchFacePineTimeStyle.h"
#include "displayapp/screens/WatchFaceNew.h"
```


Add to clock.cpp a WatchFaceNew

./src/displayapp/screens/Clock.cpp:

```cpp

std::unique_ptr<Screen> Clock::WatchFaceNewScreen() {
  return std::make_unique<Screens::WatchFaceNew>(app,
                                                     dateTimeController,
                                                     batteryController,
                                                     bleController,
                                                     notificatioManager,
                                                     settingsController,
                                                     heartRateController,
                                                     motionController);
}
```

Add to ~line 40 an additional case to select the new screen

```cpp
        case 3:
          return WatchFaceTerminalScreen();
          break;
        case 4:
          return WatchFaceNewScreen();
          break;
      }
      return WatchFaceDigitalScreen();
```

Add these reference to the head files

./src/displayapp/screens/Clock.h:

```cpp
        std::unique_ptr<Screen> WatchFacePineTimeStyleScreen();
        std::unique_ptr<Screen> WatchFaceTerminalScreen();
        std::unique_ptr<Screen> WatchFaceNewScreen();
      };
}
```

Amend the setting file to allow this watch face to be selected, by incrementing  the number of options and adding the name.

./src/displayapp/screens/settings/SettingWatchFace.h

```cpp
      private:
        static constexpr std::array<const char*, 5> options = {"Digital face", "Analog face", "PineTimeStyle", "Terminal", "New"};
        Controllers::Settings& settingsController;
```

./src/displayapp/screens/settings/SettingWatchFace.cpp

```cpp
constexpr std::array<const char*, 5> SettingWatchFace::options;

```


Add a referenace to WatchFaceAccurateWords.cpp 

./src/CMakeLists.txt:

```cpp
        ## Watch faces
        displayapp/icons/bg_clock.c
        displayapp/screens/WatchFaceAnalog.cpp
        displayapp/screens/WatchFaceDigital.cpp
        displayapp/screens/WatchFaceTerminal.cpp
        displayapp/screens/WatchFacePineTimeStyle.cpp
        displayapp/screens/WatchFaceNew.cpp
```

Then finally amened WatchFaceNew.cpp and .h with the new names rather than the old WatchFaceDigital

./src/displayapp/screens/WatchFaceNew.cpp:

```cpp
#include "displayapp/screens/WatchFaceNew.h
...
WatchFaceAccurateWords::WatchFaceNew(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
...
WatchFaceNew::~WatchFaceNew() {
  lv_task_del(taskRefresh);
...
void WatchFaceNew::Refresh() {
  statusIcons.Update();
```

./src/displayapp/screens/WatchFaceNew.cpp:

```cpp
...
  namespace Applications {
    namespace Screens {

      class WatchFaceNew : public Screen {
      public:
        WatchFaceNew(DisplayApp* app,
                         Controllers::DateTime& dateTimeController,
                         Controllers::Battery& batteryController,
                         Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificatioManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController);
        ~WatchFaceNew() override;
...
```


## Test Copied Watch Face

At this point - assuming you have [InfiniTime simulator](https://github.com/InfiniTimeOrg/InfiniSim) locally installed. It is a good time to test. 


## Amending the Watch Face to Suit 


Start editing and ammended WatchFaceNew.cpp to suit your tastes. To save space on the firmware, use existing fonts. 

## Finish

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app or Watch Face that you want to be included in InfiniTime.
