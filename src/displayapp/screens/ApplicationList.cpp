#include "ApplicationList.h"
#include <lvgl/lvgl.h>
#include <array>
#include "Symbols.h"
#include "Tile.h"
#include "displayapp/Apps.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

ApplicationList::ApplicationList(Pinetime::Applications::DisplayApp *app,
        Pinetime::Controllers::Settings &settingsController) :
        Screen(app),
        settingsController{settingsController},
        screens{app, 
          settingsController.GetAppMenu(),
          {
                [this]() -> std::unique_ptr<Screen> { return CreateScreen1(); },
                [this]() -> std::unique_ptr<Screen> { return CreateScreen2(); },
                //[this]() -> std::unique_ptr<Screen> { return CreateScreen3(); }
          },
          Screens::ScreenListModes::UpDown
        } {}


ApplicationList::~ApplicationList() {
  lv_obj_clean(lv_scr_act());
}

bool ApplicationList::Refresh() {
  if(running)
    running = screens.Refresh();
  return running;
}

bool ApplicationList::OnButtonPushed() {
  running = false;
  app->StartApp(Apps::Clock);
  return true;
}

bool ApplicationList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> ApplicationList::CreateScreen1() {
  std::array<Screens::Tile::Applications, 6> applications {
          {{Symbols::info, Apps::Notifications},
          {Symbols::music, Apps::Music},
          {Symbols::sun, Apps::Brightness},
          {Symbols::list, Apps::SysInfo},
          {Symbols::check, Apps::FirmwareValidation},
          {Symbols::heartBeat, Apps::HeartRate}
          }


  };

  return std::make_unique<Screens::Tile>(0, app, settingsController, applications);
}

std::unique_ptr<Screen> ApplicationList::CreateScreen2() {
  std::array<Screens::Tile::Applications, 6> applications {
          {{Symbols::map, Apps::Navigation},
           {Symbols::stopWatch, Apps::StopWatch},
           {Symbols::paintbrush, Apps::Paint},
           {Symbols::shoe, Apps::Motion},
           {Symbols::paddle, Apps::Paddle},
           {"2", Apps::Twos}
          }
  };

  return std::make_unique<Screens::Tile>(1, app, settingsController, applications);
}

std::unique_ptr<Screen> ApplicationList::CreateScreen3() {
  std::array<Screens::Tile::Applications, 6> applications {
          {{"A", Apps::Meter},
           {"B", Apps::Navigation},
           {"C", Apps::Clock},
           {"D", Apps::Music},
           {"E", Apps::SysInfo},
           {"F", Apps::Brightness}
          }
  };

  return std::make_unique<Screens::Tile>(2, app, settingsController, applications);
}

