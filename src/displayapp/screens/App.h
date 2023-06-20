#pragma once

#include <lvgl/lvgl.h>
#include <memory>

namespace Pinetime {
  namespace Applications {
    struct AppInterface {
      Pinetime::Components::LittleVgl* lvgl;
      Pinetime::Controllers::MotorController& motorController;
      Pinetime::Controllers::Settings& settingsController;
      Pinetime::Controllers::AlarmController& alarmController;
      Pinetime::Controllers::Timer timer;
      Pinetime::Controllers::HeartRateController& heartRateController;
      Pinetime::System::SystemTask* systemTask;
      Pinetime::Controllers::MusicService& musicService;
      Pinetime::Controllers::NavigationService& navigationService;
      // Pinetime::Controllers::WeatherService& weatherService;
    };

    namespace Screens {
      class App : Screen {
        static std::unique_ptr<Screens::Screen> Get(AppInterface appInterface);

        // returns the symbol for the app menu
        static constexpr char* GetSymbol() {
          return "?";
        };
        
        // TODO something like
        // static virtual bool NeedsMotorController() { return true; };
      };
    }
  }
}
