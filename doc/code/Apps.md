# Apps

This page will teach you:

- what screens and apps are in InfiniTime
- how to implement your own app

## Theory

The user interface of InfiniTime is made up of **screens**.
Screens that are opened from the app launcher are considered **apps**.
Every app in InfiniTime is its own class.
An instance of the class is created when the app is launched, and destroyed when the user exits the app.
Apps run inside the `DisplayApp` task (briefly discussed [here](./Intro.md)).
Apps are responsible for everything drawn on the screen when they are running.
Apps can be refreshed periodically and reacts to external events (touch or button).

## Interface

Every app class is declared inside the namespace `Pinetime::Applications::Screens` 
and inherits
from [`Pinetime::Applications::Screens::Screen`](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/src/displayapp/screens/Screen.h).

Each app defines its own constructor.
The constructors mostly take references to InfiniTime `Controllers` (ex: Alarm, DateTime, BLE services, Settings,...)
the app needs for its operations. The constructor is responsible for initializing the UI of the app.

The **destructor** cleans up LVGL and restores any changes (for example re-enable sleeping).

App classes can override `bool OnButtonPushed()`, `bool OnTouchEvent(TouchEvents event)`
and `bool OnTouchEvent(uint16_t x, uint16_t y)` to implement their own functionality for those events.

Apps that need to be refreshed periodically create an `lv_task` (using `lv_task_create()`)
that will call the method `Refresh()` periodically.

## App types

There are basically 3 types of applications : **system** apps and **user** apps and **watch faces**.

**System** applications are always built into InfiniTime, and InfiniTime cannot work properly without those apps.
Settings, notifications and the application launcher are examples of such system applications.

**User** applications are optionally built into the firmware. They extend the functionalities of the system.

**Watch faces** are very similar to the **user** apps, they are optional, but at least one must be built into the firmware.

The distinction between **system** apps,  **user** apps and watch faces allows for more flexibility and customization.
This allows to easily select which user applications and watch faces must be built into the firmware
without overflowing the system memory.

## Apps and watch faces initialization

Apps are created by `DisplayApp` in `DisplayApp::LoadScreen()`.
This method simply call the creates an instance of the class that corresponds to the app specified in parameters.

The constructor of **system** apps is called directly. If the application is a **user** app,
the corresponding `AppDescription` is first retrieved from `userApps`
and then the function `create` is called to create an instance of the app.

Watch faces are handled in a very similar way as the **user** apps : they are created by `DisplayApp` in the method `DisplayApp::LoadScreen()` when the application type is `Apps::Clock`.

## User application selection at build time

The list of user applications is generated at build time by the `consteval` function `CreateAppDescriptions()`
in `UserApps.h`. This method takes the list of applications that must be built into the firmware image.
This list of applications is defined as a list `Apps` enum values named `UserAppTypes` in `Apps.h`.
For each application listed in `UserAppTypes`, an entry of type `AppDescription` is added to the array `userApps`.
This entry is created by using the information provided by a template `AppTraits`
that is customized for every user application.

Here is an example of an AppTraits customized for the Alarm application. 
It defines the type of application, its icon and a function that returns an instance of the application.

```c++
template <>
struct AppTraits<Apps::Alarm> {
  static constexpr Apps app = Apps::Alarm;
  static constexpr const char* icon = Screens::Symbols::clock;

  static Screens::Screen* Create(AppControllers& controllers) {
    return new Screens::Alarm(controllers.alarmController,
                              controllers.settingsController.GetClockType(),
                              *controllers.systemTask,
                              controllers.motorController);
  };
};
```

This array `userApps` is used by `DisplayApp` to create the applications and the `AppLauncher`
to list all available applications.

## Watch face selection at build time

The list of available watch faces is also generated at build time by the `consteval`
function `CreateWatchFaceDescriptions()` in `UserApps.h` in the same way as the **user** apps.
Watch faces must declare a `WatchFaceTraits` so that the corresponding `WatchFaceDescription` can be generated.
Here is an example of `WatchFaceTraits`:
```c++
    template <>
    struct WatchFaceTraits<WatchFace::Analog> {
      static constexpr WatchFace watchFace = WatchFace::Analog;
      static constexpr const char* name = "Analog face";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceAnalog(controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.notificationManager,
                                            controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
```

## Creating your own app

A minimal user app could look like this:

MyApp.h:

```cpp
#pragma once

#include "displayapp/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MyApp : public Screen {
      public:
        MyApp();
        ~MyApp() override;
      };
    }
    
    template <>
    struct AppTraits<Apps:MyApp> {
      static constexpr Apps app = Apps::MyApp;
      static constexpr const char* icon = Screens::Symbol::myApp;
      static Screens::Screens* Create(AppController& controllers) {
        return new Screens::MyApp();
      }
    };
  }
}
```

MyApp.cpp:

```cpp
#include "displayapp/screens/MyApp.h"

using namespace Pinetime::Applications::Screens;

MyApp::MyApp() {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "My test application");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

MyApp::~MyApp() {
  lv_obj_clean(lv_scr_act());
}
```

Both of these files should be in [displayapp/screens/](/src/displayapp/screens/).

Now we have our very own app, but InfiniTime does not know about it yet.
The first step is to include your `MyApp.cpp` (or any new cpp files for that matter)
in the compilation by adding it to [CMakeLists.txt](/CMakeLists.txt).
The next step to making it launch-able is to give your app an id.
To do this, add an entry in the enum class `Pinetime::Applications::Apps` ([displayapp/Apps.h](/src/displayapp/Apps.h)).
Name this entry after your app. Add `#include "displayapp/screens/MyApp.h"`
to the file [displayapp/DisplayApp.cpp](/src/displayapp/DisplayApp.cpp).

If your application is a **system** application, go to the function `DisplayApp::LoadScreen`
and add another case to the switch statement.
The case will be the id you gave your app earlier.
If your app needs any additional arguments, this is the place to pass them.

If your application is a **user** application, you don't need to add anything in DisplayApp,
everything will be automatically generated for you.
The user application will also be automatically be added to the app launcher menu.

Since the list of **user** application is generated by CMake, you need to add the variable `ENABLE_USERAPPS` to the command line of CMake.
This variable must be set with a string composed of an ordered list of the **user** applications that must be built into the firmware.
The items of the list are fields from the enumeration `Apps`.
Ex : build the firmware with 3 user application : Alarm, Timer and MyApp (the application will be listed in this specific order in the application menu).

```cmake
$ cmake ... -DENABLE_USERAPPS="Apps::Alarm, Apps::Timer, Apps::MyApp" ... 
```

Similarly, the list of watch faces is also generated by CMake, so you need to add the variable `ENABLE_WATCHFACES` to the command line of CMake.
It must be set with the comma separated list of watch faces that will be built into the firmware.

Ex: build the firmware with 3 watch faces : Analog, PineTimeStyle and Infineat:

```cmake
$ cmake ... -DENABLE_WATCHFACES="WatchFace::Analog,WatchFace::PineTimeStyle,WatchFace::Infineat" ...
```

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app that you want to be included in InfiniTime.
