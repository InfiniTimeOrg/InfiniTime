#include <libs/lvgl/lvgl.h>
#include <DisplayApp/DisplayApp.h>
#include <functional>
#include "ApplicationList.h"
#include "Tile.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

ApplicationList::ApplicationList(Pinetime::Applications::DisplayApp *app) :
        Screen(app),
        screens{app, {
                [this]() -> std::unique_ptr<Screen> { return CreateScreen1(); },
                //[this]() -> std::unique_ptr<Screen> { return CreateScreen2(); },
                //[this]() -> std::unique_ptr<Screen> { return CreateScreen3(); }
          }
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
          {{Symbols::asterisk, Apps::Meter},
           {Symbols::tachometer, Apps::Gauge},
           {Symbols::clock, Apps::Clock},
           {Symbols::music, Apps::Music},
           {Symbols::list, Apps::SysInfo},
           {Symbols::sun, Apps::Brightness}
          }
  };

  return std::unique_ptr<Screen>(new Screens::Tile(app, applications));
}

std::unique_ptr<Screen> ApplicationList::CreateScreen2() {
  std::array<Screens::Tile::Applications, 6> applications {
          {{"0", Apps::Meter},
           {"1", Apps::Gauge},
           {"2", Apps::Clock},
           {"3", Apps::Music},
           {"4", Apps::SysInfo},
           {"5", Apps::Brightness}
          }
  };

  return std::unique_ptr<Screen>(new Screens::Tile(app, applications));
}

std::unique_ptr<Screen> ApplicationList::CreateScreen3() {
  std::array<Screens::Tile::Applications, 6> applications {
          {{"A", Apps::Meter},
           {"B", Apps::Gauge},
           {"C", Apps::Clock},
           {"D", Apps::Music},
           {"E", Apps::SysInfo},
           {"F", Apps::Brightness}
          }
  };

  return std::unique_ptr<Screen>(new Screens::Tile(app, applications));
}
