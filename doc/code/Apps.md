# Apps

This page will teach you:

- what screens and apps are in InfiniTime
- how to implement your own app

## Theory

The user interface of InfiniTime is made up of **screens**.
Screens that are opened from the app launcher are considered **apps**.
Every app in InfiniTime is it's own class.
An instance of the class is created when the app is launched, and destroyed when the user exits the app.
Apps run inside the "displayapp" task (briefly discussed [here](./Intro.md)).
Apps are responsible for everything drawn on the screen when they are running.
By default, apps only do something (as in a function is executed) when they are created or when a touch event is detected.

## Interface

Every app class has to be inside the namespace `Pinetime::Applications::Screens` and inherit from `Screen`.
The constructor should have at least one parameter `DisplayApp* app`, which it needs for the constructor of its parent class Screen.
Other parameters should be references to controllers that the app needs.
A destructor is needed to clean up LVGL and restore any changes (for example re-enable sleeping).
App classes can override `bool OnButtonPushed()`, `bool OnTouchEvent(TouchEvents event)` and `bool OnTouchEvent(uint16_t x, uint16_t y)` to implement their own functionality for those events.
If an app only needs to display some text and do something upon a touch screen button press,
it does not need to override any of these functions, as LVGL can also handle touch events for you.
If you have any doubts, you can always look at how the other apps function for reference.

### Continuous updating

If your app needs to be updated continuously, you can do so by overriding the `Refresh()` function in your class
and calling `lv_task_create` inside the constructor.

An example call could look like this:

```cpp
taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
```

With `taskRefresh` being a member variable of your class and of type `lv_task_t*`.
Remember to delete the task again using `lv_task_del`.
The function `RefreshTaskCallback` is inherited from `Screen` and just calls your `Refresh` function.

## Creating your own app

A minimal app could look like this:

MyApp.h:

```cpp
#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MyApp : public Screen {
      public:
        MyApp(DisplayApp* app);
        ~MyApp() override;
      };
    }
  }
}
```

MyApp.cpp:

```cpp
#include "displayapp/screens/MyApp.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

MyApp::MyApp(DisplayApp* app) : Screen(app) {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "My test application");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

MyApp::~MyApp() {
  lv_obj_clean(lv_scr_act());
}
```

Both of these files should be in [displayapp/screens/](/src/displayapp/screens/)
or [displayapp/screens/settings/](/src/displayapp/screens/settings/) if it's a setting app.

Now we have our very own app, but InfiniTime does not know about it yet.
The first step is to include your MyApp.cpp (or any new cpp files for that matter)
in the compilation by adding it to [CMakeLists.txt](/CMakeLists.txt).
The next step to making it launchable is to give your app an id.
To do this, add an entry in the enum class `Pinetime::Applications::Apps` ([displayapp/Apps.h](/src/displayapp/Apps.h)).
Name this entry after your app. Add `#include "displayapp/screens/MyApp.h"` to the file [displayapp/DisplayApp.cpp](/src/displayapp/DisplayApp.cpp).
Now, go to the function `DisplayApp::LoadScreen` and add another case to the switch statement.
The case will be the id you gave your app earlier.
If your app needs any additional arguments, this is the place to pass them.

If you want to add your app in the app launcher, add your app in [displayapp/screens/ApplicationList.h](/src/displayapp/screens/ApplicationList.h) to the array containing the applications and their corresponding symbol. If your app is a setting, do the same procedure in [displayapp/screens/settings/Settings.h](/src/displayapp/screens/settings/Settings.h).

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app that you want to be included in InfiniTime.
