#pragma once

#include <array>
#include <functional>
#include <memory>

#include "displayapp/Apps.h"
#include "displayapp/screens/Twos.h"


namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    using namespace Screens;

    struct AppInterface {
      // Pinetime::Components::LittleVgl* lvgl;
      // Pinetime::Controllers::MotorController& motorController;
      // Pinetime::Controllers::Settings& settingsController;
      // Pinetime::Controllers::AlarmController& alarmController;
      // Pinetime::Controllers::Timer timer;
      // Pinetime::Controllers::HeartRateController& heartRateController;
      // Pinetime::System::SystemTask* systemTask;
      // Pinetime::Controllers::MusicService& musicService;
      // Pinetime::Controllers::NavigationService& navigationService;
      // Pinetime::Controllers::WeatherService& weatherService;
    };

    class AppController {
    public:
      AppController();

      const char* GetSymbol(uint8_t app) const {
        app -= static_cast<uint8_t>(Apps::Dynamic);
        return symbols[app];
      };

      std::unique_ptr<Screens::Screen> Get(uint8_t app) const {
        app -= static_cast<uint8_t>(Apps::Dynamic);
        return constructors[app]();
      };

      uint8_t NApps() const {
        return current_apps;
      };

    private:
      uint8_t current_apps = 0;
      static constexpr uint8_t MAX_APP_COUNT = 24;

      std::array<const char*, MAX_APP_COUNT> symbols {};
      std::array<std::function<std::unique_ptr<Screen>()>, MAX_APP_COUNT> constructors {};
    };
  }
}
