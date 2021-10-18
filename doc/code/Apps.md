# Apps
This page will teach you:
- what apps in InfiniTime are
- how to implement your own app

## Theory
Apps are the things you can launch from the app selection you get by swiping up.
At the moment, settings and even the app launcher itself or the clock are implemented very similarly, this might change in the future though.
Every app in InfiniTime is it's own class.
An instance of the class is created when the app is launched and destroyed when the user exits the app.
They run inside the "displayapp" task (briefly discussed [here](./Intro.md)).
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
If you have any doubts, you can always look at how the other apps are doing things.

### Continuous updating
If your app needs to be updated continuously, yo can do so by overriding the `Refresh()` function in your class
and calling `lv_task_create` inside the constructor.
An example call could look like this: <br>
`taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);` <br>
With `taskRefresh` being a member variable of your class and of type `lv_task_t*`.
Remember to delete the task again using `lv_task_del`.
The function `RefreshTaskCallback` is inherited from screen and just calls your `Refresh` function.

### Apps with multiple screens
InfiniTime provides a mini-library in [displayapp/screens/ScreenList.h](/src/displayapp/screens/ScreenList.h)
which makes it relatively  easy to add multiple screens to your app.
To use it, #include it in the header file of your app and add a ScreenList member to your class.
The template argument should be the number of screens you need.
You will also need to add `CreateScreen` functions that return `std::unique_ptr<Screen>`
to your class, one for every screen you have.
There are still some things left to to that I won't cover here.
To figure them out, have a look at the "apps" ApplicationList, Settings and SystemInfo.


## Creating your own app
A minimal app could look like this: <br>
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
      }
    }
  }
}
```

MyApp.cpp:
```cpp
#include "MyApp.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

MyApp::MyApp(DisplayApp* app) : Screen(app) {
  lv_obj_t* title = lv_label_create(lv_scr_act(), NULL);  
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
Now, go to the function `DisplayApp::LoadApp` and add another case to the switch statement.
The case will be the id you gave your app earlier.
If your app needs any additional arguments, this is the place to pass them.

If you want your app to be launched from the regular app launcher, go to [displayapp/screens/ApplicationList.cpp](/src/displayapp/screens/ApplicationList.cpp).
Add your app to one of the `CreateScreen` functions, or add another `CreateScreen` function if there are no empty spaces for your app. <br>
If your app is a setting, do the same procedure in [displayapp/screens/settings/Settings.cpp](/src/displayapp/screens/settings/Settings.cpp).

You should now be able to [build](../buildAndProgram.md) the firmware
and flash it to your PineTime. Yay!

Please remember to pay attention to the [UI guidelines](../ui_guidelines.md)
when designing an app that you want to include in mainstream InfiniTime.
