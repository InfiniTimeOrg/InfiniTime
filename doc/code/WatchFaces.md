# Add Watchfaces 

This page will help you to add a new Watch Face to InfiniTime.

## Theory

This example uses the existing Digital Watch Face, copies this to a new watch face as a starting point to design a new additional watch face.

## Files 

- WatchFaceAccurateWords.cpp 
- WatchFaceAccurateWords.h 
- Clock.cpp
- Clock.h
- SettingWatchFace.h
- CMakeLists.cpp

### Check out and Branch and Copy 

Use git to fork and get a local copy (commands not shown). 

Then with all the submodules updated add a branch and copy the watch face files

```bash
git submodule update --init
git checkout -b Watchface_Accurate_Words
cp ./src/displayapp/screens/WatchFaceDigital.cpp ./src/displayapp/screens/WatchFaceAccurateWords.cpp
cp ./src/displayapp/screens/WatchFaceDigital.h ./src/displayapp/screens/WatchFaceAccurateWords.h
```

### Amend Files

Include reference to the new Watch Face

./src/displayapp/screens/Clock.cpp:

```cpp

#include "displayapp/screens/WatchFacePineTimeStyle.h"
#include "displayapp/screens/WatchFaceAccurateWords.h"
```


Add to clock.cpp a WatchFaceAccurateWordsScreen

./src/displayapp/screens/Clock.cpp:

```cpp

std::unique_ptr<Screen> Clock::WatchFaceAccurateWordsScreen() {
  return std::make_unique<Screens::WatchFaceAccurateWords>(app,
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
          return WatchFaceAccurateWordsScreen();
          break;
      }
      return WatchFaceDigitalScreen();
```

Add these reference to the head files

./src/displayapp/screens/Clock.h:

```cpp
        std::unique_ptr<Screen> WatchFacePineTimeStyleScreen();
        std::unique_ptr<Screen> WatchFaceTerminalScreen();
        std::unique_ptr<Screen> WatchFaceAccurateWordsScreen();
      };
}
```

Amend the setting file to allow this watch face to be selected, by incrementing  the number of options and adding the name.

./src/displayapp/screens/settings/SettingWatchFace.h

```cpp
      private:
        static constexpr std::array<const char*, 5> options = {"Digital face", "Analog face", "PineTimeStyle", "Terminal", "Words"};
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
        displayapp/screens/WatchFaceAccurateWords.cpp
```

Then finally amened WatchFaceAccurateWords.cpp and .h with the new names rather than the old WatchFaceDigital

./src/displayapp/screens/WatchFaceAccurateWords.cpp:

```cpp
#include "displayapp/screens/WatchFaceAccurateWords.h
...
WatchFaceAccurateWords::WatchFaceAccurateWords(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
...
WatchFaceAccurateWords::~WatchFaceAccurateWords() {
  lv_task_del(taskRefresh);
...
void WatchFaceAccurateWords::Refresh() {
  statusIcons.Update();
```

./src/displayapp/screens/WatchFaceAccurateWords.cpp:

```cpp
...
  namespace Applications {
    namespace Screens {

      class WatchFaceAccurateWords : public Screen {
      public:
        WatchFaceAccurateWords(DisplayApp* app,
                         Controllers::DateTime& dateTimeController,
                         Controllers::Battery& batteryController,
                         Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificatioManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController);
        ~WatchFaceAccurateWords() override;
...
```


## Test Copied Watch Face

At this point - assuming you have [InfiniTime simulator](https://github.com/InfiniTimeOrg/InfiniSim) locally installed. It is a good time to test. 


## Amending the Watch Face to Suit 


Start editing and ammended WatchFaceAccurateWords.cpp to suit your tastes. To save space on the firmware, use existing fonts. 

## Finish

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app or Watch Face that you want to be included in InfiniTime.
