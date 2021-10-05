# Apps
This page will teach you:
- what apps in InfiniTime are
- how to implement your own app

## Theory
Apps are the things you can lauch from the app selection you get by swiping down, but also the app launcher itself or the clock. Settings are also implemented as apps. Most screens you see are their own app, except for apps that have multiple screens (settings launcher, app launcher).
Every app in InfiniTime is it's own class. An object of the class is created when the app is launched and destroyed when the user exits the app. They run inside the "displayapp" task (briefly discussed [here](./Intro.md)). They are responsible for everything drawn on the screen when they are running. By default, apps only do something (as in a function is executed) when they are created or when a touch event is detected.

## Interface
Every app class is has to be inside the namespace `Pinetime::Applications::Screens` and inherit from `Screen`. The constructor should have at least one parameter `DisplayApp* app`, which it needs for the constructor of its parent class Screen. Other parameters should be references to controllers that the app needs. A deconstructor is needed to clean up LVGL and restore any changes (for example re-enable sleeping). App classes can override `bool OnButtonPushed()`, `bool OnTouchEvent(TouchEvents event)` and `bool OnTouchEvent(uint16_t x, uint16_t y)` to implement their own functionality for those events. If an app only needs to display some text and do something upon a touch screen button press, it does not need to override any of these functions, as LVGL can also handle touch events for you. If your app needs to be updated continuously, yo can do so by adding a `Refresh()` function to your class and calling `lv_task_create` inside the constructor. 

## Creating your own app
A minimal app could look like this: <br>
MyApp.h:
```cpp
#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"

namespace PineTime {
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
    lv_obj_t * container1 = lv_cont_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_pos(container1, 30, 60);
    lv_obj_set_width(container1, LV_HOR_RES - 50);
    lv_obj_set_height(container1, LV_VER_RES - 60);
    lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

    lv_obj_t * title = lv_label_create(lv_scr_act(), NULL);  
    lv_label_set_text_static(title,"My test application");
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);
}

MyApp::~MyApp() {
    lv_obj_clean(lv_scr_act());
}
```
Both of these files should be in displayapp/screens/ or displayapp/screens/settings if it's a setting app.

Now we have our very own app, but InfiniTime does not know about it yet. The first step is to include it in the compilation by adding it to CMakeLists.txt . The next step to making it launchable is to give your app an id. To do this, add an entry in the enum class `Pinetime::Applications::Apps`. Name this entry after your app. Add `#include "displayapp/screens/MyApp.h"` to the file displayapp/DisplayApp.cpp . Now, go to the function `DisplayApp::LoadApp` and add another case to the switch statement. The case will be the id you gave your app earlier. If your app needs any additional arguments, this is the place to pass them. <br>

If you want your app to be launched from the regular app launcher, go to displayapp/screens/ApplicationList.cpp. Add your app to one of the `CreateScreen` functions, or add another `CreateScreen` function if there are no empty spaces for your app. <br>
If your app is a setting, do the same procedure in displayapp/screens/settings/Settings.cpp .

You should now be able to [build](../buildAndProgram.md) the firmware and flash it to your PineTime. Yay!
