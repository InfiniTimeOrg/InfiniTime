#pragma once

namespace Pinetime {
  namespace Applications {
    class DisplayApp;
  }

  namespace Components {
    class LittleVgl;
  }

  namespace Controllers {
    class Battery;
    class Ble;
    class DateTime;
    class NotificationManager;
    class HeartRateController;
    class Settings;
    class MotorController;
    class MotionController;
    class AlarmController;
    class BrightnessController;
    class WeatherService;
    class FS;
    class Timer;
    class MusicService;
    class NavigationService;
  }

  namespace System {
    class SystemTask;
  }

  namespace Applications {
    struct AppControllers {
      const Pinetime::Controllers::Battery& batteryController;
      const Pinetime::Controllers::Ble& bleController;
      Pinetime::Controllers::DateTime& dateTimeController;
      Pinetime::Controllers::NotificationManager& notificationManager;
      Pinetime::Controllers::HeartRateController& heartRateController;
      Pinetime::Controllers::Settings& settingsController;
      Pinetime::Controllers::MotorController& motorController;
      Pinetime::Controllers::MotionController& motionController;
      Pinetime::Controllers::AlarmController& alarmController;
      Pinetime::Controllers::BrightnessController& brightnessController;
      Pinetime::Controllers::WeatherService* weatherController;
      Pinetime::Controllers::FS& filesystem;
      Pinetime::Controllers::Timer& timer;
      Pinetime::System::SystemTask* systemTask;
      Pinetime::Applications::DisplayApp* displayApp;
      Pinetime::Components::LittleVgl& lvgl;
      Pinetime::Controllers::MusicService* musicService;
      Pinetime::Controllers::NavigationService* navigationService;
    };
  }
}
